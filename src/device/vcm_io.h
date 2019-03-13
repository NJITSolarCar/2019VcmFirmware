/*
 * vcm_io.h
 *
 * Primary set of drivers for the VCM in terms of I/O. This provides
 * the external API of the VCM itself, particularly over CAN. This is not
 * so much the place for program flow control, structure, and algorithm. That
 * should be resident in main.
 *
 *  Created on: Mar 12, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_VCM_IO_H_
#define SRC_DEVICE_VCM_IO_H_

/**
 * Initializes the vcm I/O system. This means just setting up internal
 * communications and CAN infrastructure. This doesn't include CVNP
 * initialization, but it should include binding different handlers
 * and other implementation specific details.
 */
vcmio_init();

#endif /* SRC_DEVICE_VCM_H_ */
