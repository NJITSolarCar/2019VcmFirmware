/*
 * debug_led.h
 *
 * Code for the onboard debug LEDs and switches
 *
 *  Created on: Apr 23, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_DEBUG_LED_H_
#define SRC_DEVICE_DEBUG_LED_H_

#define DEBUGLED_LOAD				40000000

// Initializes the debug LED system
void debugLed_init();

// Sets the LED to a blinking pattern
void debugLed_setBlink();

// Sets the LED to a solid pattern
void debugLed_setSolid(bool on);

#endif /* SRC_DEVICE_DEBUG_LED_H_ */
