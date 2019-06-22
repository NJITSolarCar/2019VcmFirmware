/*
 * relay.c
 *
 *  Created on: Apr 19, 2019
 *      Author: Duemmer
 */

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/gpio.h>

#include "relay.h"
#include "../fault.h"
#include "../hal/resource.h"

// Relay master enable flag
static bool g_relayEnable = false;

// Relay override enable flag
static bool g_relayOverride = false;

// Individual relay state flags
static bool g_battPlusState = false;
static bool g_battMinusState = false;
static bool g_dischargeState = false;
static bool g_chargeState = false;
static bool g_solarState = false;


/**
 * Updates all relay states based on the relay status flags
 */
void _relay_updateAll() {
	relay_setBattPlus(g_battPlusState);
	relay_setBattMinus(g_battMinusState);
	relay_setDischarge(g_dischargeState);
	relay_setCharge(g_chargeState);
	relay_setSolar(g_solarState);
}



void relay_init() {
	// Set up relays
	GPIOPinTypeGPIOOutput(CHG_RLY_PORT, CHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(DISCHG_RLY_PORT, DISCHG_RLY_PIN);
	GPIOPinTypeGPIOOutput(SOLAR_RLY_PORT, SOLAR_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATPOS_RLY_PORT, BATPOS_RLY_PIN);
	GPIOPinTypeGPIOOutput(BATNEG_RLY_PORT, BATNEG_RLY_PIN);
}


/**
 * Relay controls
 */
void relay_setBattPlus(bool on) {
	g_battPlusState = on;
	uint8_t outState = g_relayOverride || (on && g_relayEnable) ? 0xFF : 0;
	GPIOPinWrite(BATPOS_RLY_PORT, BATPOS_RLY_PIN, outState);
}


void relay_setBattMinus(bool on) {
	g_battMinusState = on;
	uint8_t outState =  g_relayOverride || (on && g_relayEnable) ? 0xFF : 0;
	GPIOPinWrite(BATNEG_RLY_PORT, BATNEG_RLY_PIN, outState);
}


void relay_setDischarge(bool on) {
	g_dischargeState = on;
	uint8_t outState =  g_relayOverride || (on && g_relayEnable) ? 0xFF : 0;
	GPIOPinWrite(DISCHG_RLY_PORT, DISCHG_RLY_PIN, outState);
}


void relay_setCharge(bool on) {
	g_chargeState = on;
	uint8_t outState =  g_relayOverride || (on && g_relayEnable) ? 0xFF : 0;
	GPIOPinWrite(CHG_RLY_PORT, CHG_RLY_PIN, outState);
}


void relay_setSolar(bool on) {
	g_solarState = on;
	uint8_t outState =  g_relayOverride || (on && g_relayEnable) ? 0xFF : 0;
	GPIOPinWrite(SOLAR_RLY_PORT, SOLAR_RLY_PIN, outState);
}


void relay_setAll(bool on) {
	g_battMinusState = on;
	g_battPlusState = on;
	g_chargeState = on;
	g_dischargeState = on;
	g_solarState = on;

	uint8_t outState = on && g_relayEnable ? 0xFF : 0;
	GPIOPinWrite(BATPOS_RLY_PORT, BATPOS_RLY_PIN, outState);
	GPIOPinWrite(BATNEG_RLY_PORT, BATNEG_RLY_PIN, outState);
	GPIOPinWrite(DISCHG_RLY_PORT, DISCHG_RLY_PIN, outState);
	GPIOPinWrite(CHG_RLY_PORT, CHG_RLY_PIN, outState);
	GPIOPinWrite(SOLAR_RLY_PORT, SOLAR_RLY_PIN, outState);
}




/**
 * Master enable for relays. Acts as a mask with
 * the values passed to relay_set*(). If enable is set
 * to true, the relays will operate normally. If not, relays can still
 * be set, but will not turn on until true is passed.
 */
void relay_enable(bool enable) {
	g_relayEnable = enable;
	_relay_updateAll();
}




/**
 * Relay statuses
 */
bool relay_getBattPlus() {
	return !!GPIOPinRead(BATPOS_RLY_PORT, BATPOS_RLY_PIN);
}


bool relay_getBattMinus() {
	return !!GPIOPinRead(BATNEG_RLY_PORT, BATNEG_RLY_PIN);
}


bool relay_getDischarge() {
	return !!GPIOPinRead(DISCHG_RLY_PORT, DISCHG_RLY_PIN);
}


bool relay_getCharge() {
	return !!GPIOPinRead(CHG_RLY_PORT, CHG_RLY_PIN);
}


bool relay_getSolar() {
	return !!GPIOPinRead(SOLAR_RLY_PORT, SOLAR_RLY_PIN);
}



void relay_override(bool enable) {
	g_relayOverride = enable;
}




/**
 * Sets all relays according to the faults set at the
 * time of calling
 *
 * TODO: A potential race condition exists with this method and the
 * fault system. If the fault status is read, and an interrupt that asserts
 * a fault blocks the relay updates, it is possible for a relay to stay enabled
 * when it should be cleared. As such, this function should be used sparingly, if
 * at all.
 */
void relay_setFromFaults() {
	// TODO: develop a fault-relay table and use that
}





