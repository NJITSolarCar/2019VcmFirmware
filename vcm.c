/*
 * vcm.c
 *
 * Primary control algorithm for the VCM. This file contains the
 * main flow control loop of the system, as well as high level
 * scheduling and module control. Basic flow execution of the VCM
 * is as follows:
 *
 * On reset,the following startup sequence will be executed:
 * - Configure MCU peripherals
 * - Bind ISRs
 * - Assert all I/Os into a safe default state
 * - Enable timestamp timer
 * - Perform a Power On Self Test (POST) of the entire vehicle system
 * 		- If any errors, assert relevant comm / POST faults, set indicator, and trap the VCM
 * - Enable timing loops
 * - Start watchdog
 * - Enter Run Mode
 *
 * While in Run Mode:
 * - Idle loop: Do nothing (as of now)
 * - Timing loop: Continuously run periodic tick() routines
 *
 * At any point, certain interrupts can change the regular control flow, and
 * their specific behavior can be detailed at the ISR.
 *
 *  Created on: April 1, 2019
 *      Author: Duemmer
 */



// Dummy include to prevent a compile error occurring in this header file
//#include "src/hal/resource.h"

#include "stdbool.h"
#include "stdint.h"

/**
 * main.c
 */
int main(void)
{
	return 0;
}
