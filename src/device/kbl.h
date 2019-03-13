/*
 * kbl.h
 *
 * Driver for the motor controller. The functions listed should be enough
 * for the external interface, but many more functions will need to be
 * written to be able to get information from the motor controller
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_H_KBL_H_
#define SRC_DEVICE_H_KBL_H_

#include <stdint.h>

/**
 * Status information about a motor and motor controller
 */
typedef struct {
    float ia, ib, ic;                       // Phase currents
    float iMot                              // Battery / motor current
    float va, vb, vc;                       // Phase voltages
    float vBat                              // Battery voltage
    float vOperating;                       // 12V rail voltage
    float brake;                             // Brake percentage
    float throttle;                         // Throttle percentage
    float mechRPM;                          // Mechanical RPM
    float tMot;                             // Motor temperature
    float tController;                      // Controller temperature
    uint8_t swThrottle : 1;                 // Throttle switch state
    uint8_t swBrake : 1;                    // Brake switch state
    uint8_t swRev : 1;                      // Reverse switch state
} tMotorData;


/**
 * Initializes the motor controller driver
 */
void kbl_init();


/**
 * Tick routine to be run every ~10ms. This is where this api can
 * periodically query, check stuff, etc.
 *
 * \param now the current millisecond timestamp
 */
void kbl_tick(uint32_t now);


/**
 * Retrieves the instantaneous data for both motors, or 0 if the data is invalid.
 * The structures pointed to by the return pointer are persistent,
 * regardless of stack state.
 */
tMotorData *kbl_leftMotData()
tMotorData *kbl_rightMotData();

#endif /* SRC_DEVICE_H_KBL_H_ */






