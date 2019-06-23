/*
 * gpio.c
 *
 * Main GPIO interface drivers for the VCM
 *
 *  Created on: Apr 10, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>

#include <driverlib/timer.h>
#include <driverlib/gpio.h>

#include "resource.h"
#include "gpio.h"

#include "../fault.h"

#include "../device/indicator.h"
#include "../device/bms.h"
#include "../device/relay.h"
#include "../device/mppt.h"

#include "../util.h"

// ms timestamps of the last registered interrupt on each pin
static uint32_t g_lastIgnitionChg;
static uint32_t g_lastMpptUserSwChg;
static uint32_t g_lastRelayForceChg;

// Last valid states of the pins
static bool g_lastIgnitionState;
static bool g_lastMpptUserSwState;
static bool g_lastRelayForceState;

// Ignition state of the vehicle
static bool g_ignitionActive;




static inline void _gpio_ignitionChg(bool isOn) {
	g_ignitionActive = !isOn;
	if(!isOn) { // Switch logic is inverted
		relay_enable(true);
		if(!fault_getFaultSummary())
			indicator_setPattern(LED_STAT_NOFLT_ENBL);
	} else {
		relay_enable(false);
		if(!fault_getFaultSummary())
			indicator_setPattern(LED_STAT_NOFLT_DISBL);
	}
}

static inline void _gpio_mpptUserSwChg(bool isOn) {
	if(!isOn) { // Switch logic is inverted
		tFaultData dat;
		dat.ui64 = 0;
		fault_assert(FAULT_MPPT_USER_LOCKOUT, dat);
	} else
		fault_deassert(FAULT_MPPT_USER_LOCKOUT);
}

static inline void _gpio_relayForceChg(bool isOn) {
	if(!isOn) { // Switch logic is inverted
		tFaultData dat;
		dat.ui64 = 0;
		fault_assert(FAULT_RELAY_OVERRIDE, dat);
	} else
		fault_deassert(FAULT_RELAY_OVERRIDE);
}



void _gpio_intPortF() {
	uint32_t intStat = GPIOIntStatus(GPIO_PORTF_BASE, true);
	uint8_t pinState = GPIOPinRead(GPIO_PORTF_BASE, 0xFF);
	uint32_t now = util_msTimestamp();

	GPIOIntClear(GPIO_PORTF_BASE, 0xFF);

	if(intStat & BMS_DCL_PIN) {
		bms_onDCLGpioChange(pinState & BMS_DCL_PIN);
	}
}


void _gpio_intPortB() {
	uint32_t intStat = GPIOIntStatus(GPIO_PORTB_BASE, true);
	uint8_t pinState = GPIOPinRead(GPIO_PORTB_BASE, 0xFF);
	uint32_t now = util_msTimestamp();

	GPIOIntClear(GPIO_PORTB_BASE, 0xFF);

	if(intStat & DASH_MPPT_PIN)
		g_lastMpptUserSwChg = now;

	if(intStat & DASH_RELAY_FORCE_PIN)
		g_lastRelayForceChg = now;

	if(intStat & DASH_IGNITION_PIN)
		g_lastIgnitionChg = now;
}


void gpio_init() {

	// Set up port F ISRs
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, BMS_DCL_PIN);
	GPIOIntEnable(GPIO_PORTF_BASE, BMS_DCL_PIN);
	GPIOIntTypeSet(GPIO_PORTF_BASE, BMS_DCL_PIN, GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTF_BASE, _gpio_intPortF);

	// Set up port B ISRs
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, DASH_MPPT_PIN | DASH_IGNITION_PIN | DASH_RELAY_FORCE_PIN);
	GPIOIntEnable(GPIO_PORTB_BASE, DASH_MPPT_PIN | DASH_IGNITION_PIN | DASH_RELAY_FORCE_PIN);
	GPIOIntTypeSet(GPIO_PORTB_BASE, DASH_MPPT_PIN | DASH_IGNITION_PIN | DASH_RELAY_FORCE_PIN, GPIO_BOTH_EDGES);
	GPIOIntRegister(GPIO_PORTB_BASE, _gpio_intPortB);

	// Set initial states
	g_lastIgnitionState = !!GPIOPinRead(DASH_IGNITION_PORT, DASH_IGNITION_PIN);
	g_lastMpptUserSwState = !!GPIOPinRead(DASH_MPPT_PORT, DASH_MPPT_PIN);
	g_lastRelayForceState = !!GPIOPinRead(DASH_RELAY_FORCE_PORT, DASH_RELAY_FORCE_PIN);
}



void gpio_tick() {
	uint32_t now = util_msTimestamp();

	// Handle an ignition change
	if(g_lastIgnitionState !=
			!!GPIOPinRead(DASH_IGNITION_PORT, DASH_IGNITION_PIN) &&
			now-g_lastIgnitionChg > GPIO_INPUT_HOLD_MS) {
		g_lastIgnitionState ^= 1; // State has toggled, so flip the flag accordingly
		_gpio_ignitionChg(g_lastIgnitionState);
	}

	// Handle an mppt user switch change
	if(g_lastMpptUserSwState !=
			!!GPIOPinRead(DASH_MPPT_PORT, DASH_MPPT_PIN) &&
			now-g_lastMpptUserSwChg > GPIO_INPUT_HOLD_MS) {
		g_lastMpptUserSwState ^= 1; // State has toggled, so flip the flag accordingly
		_gpio_mpptUserSwChg(g_lastMpptUserSwState);
	}

	// Handle a relay override switch change
	if(g_lastRelayForceState !=
			!!GPIOPinRead(DASH_RELAY_FORCE_PORT, DASH_RELAY_FORCE_PIN) &&
			now-g_lastRelayForceChg > GPIO_INPUT_HOLD_MS) {
		g_lastRelayForceState ^= 1; // State has toggled, so flip the flag accordingly
		_gpio_relayForceChg(g_lastRelayForceState);
	}
}



bool gpio_ignitionActive() {
	return g_ignitionActive;
}









