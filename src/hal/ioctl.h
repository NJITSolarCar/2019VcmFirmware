/*
 * ioctl.h
 *
 * System level configuration for the microcontroller. This doesn't really
 * operate on outputs and I/O so much as configures clocking, FPU, etc.
 *
 *  Created on: Mar 8, 2019
 *      Author: Duemmer
 */

#ifndef SRC_HAL_IOCTL_H_
#define SRC_HAL_IOCTL_H_

// Clock settings, configured for 80 MHz. See datasheet page 208 for divider reference
#define IOCTL_MCU_XTAL				SYSCTL_XTAL_8MHZ
#define IOCTL_MCU_VCO				SYSCTL_CFG_VCO_480
#define IOCTL_MCU_PLL_DIV			SYSCTL_SYSDIV_2_5

#define IOCTL_WDT_LOAD				40000000 // 0.5s

/**
 * Configures the entire system to prepare for the rest of initialization.
 * sets clocks, FPU, etc. to put the system in a known state.
 */
void ioctl_reset();

/**
 * Reloads the watchdog timer. Must be called periodically by the system.
 */
void ioctl_loadWatchdog();

#endif /* SRC_HAL_IOCTL_H_ */
