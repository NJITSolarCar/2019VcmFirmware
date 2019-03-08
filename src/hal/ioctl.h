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

/**
 * Configures the entire system to prepare for the rest of initialization.
 * sets clocks, FPU, etc. to put the system in a known state.
 */
ioctl_reset();

#endif /* SRC_HAL_IOCTL_H_ */
