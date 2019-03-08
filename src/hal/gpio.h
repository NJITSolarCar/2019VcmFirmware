/*
 * gpio.h
 *
 * Contains all HAL functions for configuring, interfacing, and
 * using basic GPIO
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

/**
 * Initializes the pins used as GPIOs.
 */
void gpio_init();


/**
 * Relay controls
 */
void gpio_setRelayBattPlus(bool on);
void gpio_setRelayBattMinus(bool on);
void gpio_setRelayDischarge(bool on);
void gpio_setRelayCharge(bool on);
void gpio_setRelaySolar(bool on);


#endif /* HAL_GPIO_H_ */
