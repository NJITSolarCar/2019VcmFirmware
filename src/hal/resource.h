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
#include <driverlib/can.h>

#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>

// Internal
#define SYS_US_TIMER_BASE				WTIMER0_BASE
#define SYS_WATCHDOG					WATCHDOG0_BASE

/*
 * DASH / INDICATOR
 */
#define DASH_IGNITION_PIN				GPIO_PIN_1
#define DASH_IGNITION_PORT				GPIO_PORTB_BASE

#define DASH_MPPT_PORT					GPIO_PORTB_BASE
#define DASH_MPPT_PIN					GPIO_PIN_2

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
#define INA_GS_PORT						GPIO_PORTC_BASE
#define INA_GS0_PIN						GPIO_PIN_5
#define INA_GS1_PIN						GPIO_PIN_4

#define INA_ADC_PORT					GPIO_PORTD_BASE
#define INA_ADC_PIN						GPIO_PIN_0
#define INA_ADC_CHANNEL					ADC_CTL_CH7

#define INA_ADC_MODULE					ADC0_BASE
#define INA_ADC_SEQUENCE				3
#define INA_ADC_PRIORITY				3

#define INA_TIMER_BASE					TIMER0_BASE
#define INA_TIMER_PART					TIMER_A
#define INA_TIMER_INT					TIMER_TIMA_TIMEOUT
#define INA_TIMER_CFG					TIMER_CFG_A_ONE_SHOT

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

#define THERM_ADC_MODULE				ADC1_BASE
#define THERM_ADC_SEQUENCE				2

// CAN
#define CAN_RX_PORT						GPIO_PORTE_BASE
#define CAN_RX_PIN						GPIO_PIN_4
#define CAN_RX_PINCONFIG				GPIO_PE4_CAN0RX

#define CAN_TX_PORT						GPIO_PORTE_BASE
#define CAN_TX_PIN						GPIO_PIN_5
#define CAN_TX_PINCONFIG				GPIO_PE5_CAN0TX

#define CAN_STB_PORT					GPIO_PORTB_BASE
#define CAN_STB_PIN						GPIO_PIN_4

#define CAN_RX_IFACE					CAN0_BASE
#define CAN_TX_IFACE					CAN1_BASE

// BMS
#define BMS_DCL_PORT					GPIO_PORTF_BASE
#define BMS_DCL_PIN						GPIO_PIN_3

#endif /* SRC_HAL_RESOURCE_H_ */





