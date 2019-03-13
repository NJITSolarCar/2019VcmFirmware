/*
 * resource.h
 *
 * Contains a defined list of all HAL resources and bindings. This includes GPIO pins, ports,
 * timers, ADCs, etc. This serves only as a bridge, in the form of HAL constants (ports, pins, etc.)
 * to VCM resources.
 *
 * Resource allocation:
 * DASH:
 * 		GPIO: PB0, PB1, PB2, PB3, PG0, PG1, PG2
 * 		Timer: T5, T4, WT0A
 *
 *  Created on: Mar 12, 2019
 *      Author: Duemmer
 */

#ifndef SRC_HAL_RESOURCE_H_
#define SRC_HAL_RESOURCE_H_

#include <stdint.h>
#include <stdbool.h>

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/adc.h>
#include <driverlib/timer.h>

#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>

/*
 * DASH / INDICATOR
 */
#define DASH_IGNITION_PIN				GPIO_PIN_6
#define DASH_IGNITION_PORT				GPIO_PORTB_BASE

// indicator LED
#define DASH_LED_RED_PIN				GPIO_PIN_2
#define DASH_LED_GREEN_PIN				GPIO_PIN_1
#define DASH_LED_BLUE_PIN				GPIO_PIN_0
#define DASH_LED_PORT					GPIO_PORTG_BASE
#define DASH_LED_RED_PINMAP				GPIO_PG2_T5CCP0
#define DASH_LED_GREEN_PINMAP			GPIO_PG1_T4CCP1
#define DASH_LED_BLUE_PINMAP			GPIO_PG0_T4CCP0

#define DASH_LED_BLINK_TIMER_BASE		WTIMER0_BASE
#define DASH_LED_BLINK_TIMER_PART		TIMER_A

#endif /* SRC_HAL_RESOURCE_H_ */
