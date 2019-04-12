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

#define DASH_LED_BLINK_TIMER_BASE		TIMER1_BASE

// Relays
#define CHG_RLY_PORT					GPIO_PORTA_BASE
#define CHG_RLY_PIN						GPIO_PIN_2

#define DISCHG_RLY_PORT					GPIO_PORTA_BASE
#define DISCHG_RLY_PIN					GPIO_PIN_3

#define SOLAR_RLY_PORT					GPIO_PORTA_BASE
#define SOLAR_RLY_PIN					GPIO_PIN_4

#define BATPOS_RLY_PORT					GPIO_PORTA_BASE
#define BATPOS_RLY_PIN					GPIO_PIN_5

#define BATNEG_RLY_PORT					GPIO_PORTA_BASE
#define BATNEG_RLY_PIN					GPIO_PIN_6

// INA225
#define INA_GS0_PORT					GPIO_PORTC_BASE
#define INA_GS0_PIN						GPIO_PIN_5

#define INA_GS1_PORT					GPIO_PORTC_BASE
#define INA_GS1_PIN						GPIO_PIN_4

#define INA_ADC_PORT					GPIO_PORTD_BASE
#define INA_ADC_PIN						GPIO_PIN_0
#define INA_ADC_CHANNEL					ADC_CTL_CH7

// Thermistors
#define THERM1_PORT						GPIO_PORTE_BASE
#define THERM1_PIN						GPIO_PIN_3
#define THERM1_ADC_CHANNEL				ADC_CTL_CH0

#define THERM2_PORT						GPIO_PORTE_BASE
#define THERM2_PIN						GPIO_PIN_2
#define THERM2_ADC_CHANNEL				ADC_CTL_CH1

#define THERM3_PORT						GPIO_PORTE_BASE
#define THERM3_PIN						GPIO_PIN_1
#define THERM3_ADC_CHANNEL				ADC_CTL_CH2

#endif /* SRC_HAL_RESOURCE_H_ */
