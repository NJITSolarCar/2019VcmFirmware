/*
 * debug_led.c
 *
 *  Created on: Apr 23, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>

#include <driverlib/gpio.h>
#include <driverlib/timer.h>

#include "indicator.h"
#include "debug_led.h"

#include "../hal/resource.h"

static bool g_lastState;

void _debugLed_timerInt() {
	TimerIntClear(DEBUG_LED_TIMER, TIMER_TIMA_TIMEOUT);
	g_lastState ^= 1;
	GPIOPinWrite(DEBUG_LED_PORT, DEBUG_LED_PIN, g_lastState ? 0xFF : 0);
}


void debugLed_setBlink() {
	TimerIntEnable(DEBUG_LED_TIMER, TIMER_TIMA_TIMEOUT);
}


void debugLed_setSolid(bool on) {
	TimerIntDisable(DEBUG_LED_TIMER, TIMER_TIMA_TIMEOUT);
	GPIOPinWrite(DEBUG_LED_PORT, DEBUG_LED_PIN, on ? 0xFF : 0);
}


void debugLed_init() {

	// Set GPIO as output
	GPIOPinTypeGPIOOutput(DEBUG_LED_PORT, DEBUG_LED_PIN);

	// Setup the blink timer
	TimerConfigure(DEBUG_LED_TIMER, TIMER_CFG_PERIODIC);
	TimerLoadSet(DEBUG_LED_TIMER, TIMER_A, DEBUGLED_LOAD);
	TimerIntRegister(DEBUG_LED_TIMER, TIMER_A, _debugLed_timerInt);
	TimerIntEnable(DEBUG_LED_TIMER, TIMER_TIMA_TIMEOUT);
	TimerEnable(DEBUG_LED_TIMER, TIMER_A);

	// Default to GPIO off
	debugLed_setSolid(false);
}













