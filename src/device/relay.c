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
	GPIOPinWrite(BATPOS_RLY_PORT, BATPOS_RLY_PIN, on ? 0xFF : 0);
}


void relay_setBattMinus(bool on) {
	GPIOPinWrite(BATNEG_RLY_PORT, BATNEG_RLY_PIN, on ? 0xFF : 0);
}


void relay_setDischarge(bool on) {
	GPIOPinWrite(DISCHG_RLY_PORT, DISCHG_RLY_PIN, on ? 0xFF : 0);
}


void relay_setCharge(bool on) {
	GPIOPinWrite(CHG_RLY_PORT, CHG_RLY_PIN, on ? 0xFF : 0);
}


void relay_setSolar(bool on) {
	GPIOPinWrite(SOLAR_RLY_PORT, SOLAR_RLY_PIN, on ? 0xFF : 0);
}


void relay_setAll(bool on) {
	uint8_t outState = on ? 0xFF : 0;
	GPIOPinWrite(BATPOS_RLY_PORT, BATPOS_RLY_PIN, outState);
	GPIOPinWrite(BATNEG_RLY_PORT, BATNEG_RLY_PIN, outState);
	GPIOPinWrite(DISCHG_RLY_PORT, DISCHG_RLY_PIN, outState);
	GPIOPinWrite(CHG_RLY_PORT, CHG_RLY_PIN, outState);
	GPIOPinWrite(SOLAR_RLY_PORT, SOLAR_RLY_PIN, outState);
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








