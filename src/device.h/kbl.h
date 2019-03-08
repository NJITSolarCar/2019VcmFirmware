/*
 * kbl.h
 *
 * Driver for the motor controller
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_H_KBL_H_
#define SRC_DEVICE_H_KBL_H_

/**
 * Initializes the motor controller driver
 */
void kbl_init();


/**
 * Queries the motor controllers for ADC Batch 1
 */
void kbl_queryAdcBatch1();

#endif /* SRC_DEVICE_H_KBL_H_ */
