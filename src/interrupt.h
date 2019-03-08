/*
 * interrupt.h
 *
 * Contains all of the interrupt service routines for the VCM. HAL
 * routines will bind these to different hardware vectors
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

/**
 * ADC interrupts. These are called when ADC samples are completed
 */


#endif /* SRC_INTERRUPT_H_ */
