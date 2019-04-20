/*
 * kbl.h
 *
 * Driver for the motor controller. The functions listed should be enough
 * for the external interface, but many more functions will need to be
 * written to be able to get information from the motor controller.
 *
 * Peripherals used:
 * - Timer 0
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_H_KBL_H_
#define SRC_DEVICE_H_KBL_H_

#include <stdint.h>

// How many 10 ms ticks elapse between successive queries. Will query L, wait
// this many ticks, query R, wait this many ticks, etc.
#define KBL_TICK_UPDATE_PERIOD				5

// Frame IDs for the left and right motor
#define KBL_ID_TX_LEFT						0x70
#define KBL_ID_TX_RIGHT						0x71
#define KBL_ID_RX_LEFT						0x72
#define KBL_ID_RX_RIGHT						0x73

// Query type bytes
#define KBL_FRAME_A2D_BATCH_1				0x1B
#define KBL_FRAME_A2D_BATCH_2				0x1A
#define KBL_FRAME_CCP1						0x33
#define KBL_FRAME_CCP2						0x37
#define KBL_FRAME_COM_SW_ACC				0x42
#define KBL_FRAME_COM_SW_REV				0x44
#define KBL_FRAME_COM_SW_BRK				0x43

#define KBL_VBAT_SCL						0.369 // 1/2.71
#define KBL_THROTTLE_SCL					0.0196078f // 5/255
#define KBL_VS_TO_VOLTS(x)					((((float)(x-120))*0.0357f)+4.75f) // Conversion of hall sensor voltage from ADC reading to an analog voltage

#define KBL_RATED_CURRENT					150.0f
#define KBL_NUM_POLES_INV					0.083333f // 1/12

/**
 * Status information about a motor and motor controller
 */
typedef struct {
    float ia, ib, ic;                       // Phase currents
    float iMot;                              // Battery / motor current
    float va, vb, vc;                       // Phase voltages
    float vBat;                              // Battery voltage
	float vs;								// Hall sensor voltage
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
 * Tick routine to be run every 10ms. This is where this api can
 * periodically query, check stuff, etc.
 */
void kbl_tick();


/**
 * Retrieves the instantaneous data for both motors, or 0 if the data is invalid.
 * The structures pointed to by the return pointer are persistent,
 * regardless of stack state.
 */
tMotorData *kbl_leftMotData();
tMotorData *kbl_rightMotData();


#endif /* SRC_DEVICE_H_KBL_H_ */






