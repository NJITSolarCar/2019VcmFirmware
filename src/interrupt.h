/*
 * interrupt.h
 *
 * Contains all of the interrupt service routines for the VCM. HAL
 * routines will bind these to different hardware vectors.
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef SRC_INTERRUPT_H_
#define SRC_INTERRUPT_H_

/**
 * GPIO Input handlers. These will be called when certain I/O lines rise
 * and / or fall. NOTE: for these Routines, it can be assumed that the interrupt
 * flag will have already been cleared.
 */
void int_onEstopChange(bool newState);
void int_onLeftMotLEDChange(bool newState);
void int_onRightMotLEDChange(bool newState);
void int_onIgnitionPress();
void int_onRunSwitchChange(bool newState);
void int_onMpptSwitchChange(bool newState);



// ADC interrupts. These are called when ADC samples are completed,
// and contain the data of the sample.

/**
 * Called when a sample of the 3 onboard thermistor values is complete.
 * The buffer contains the 3 samples, in order.
 */
void int_onThermoSampleDone(uint32_t *pBuf);

/**
 * Called when the sample across the current shunt is competed, and provides
 * the value of the sample.
 */
void int_onCurrentSampleDone(uint32_t adc);



void int_


#endif /* SRC_INTERRUPT_H_ */
