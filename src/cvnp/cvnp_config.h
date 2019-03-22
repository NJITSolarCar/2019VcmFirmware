/*
 * cvnp_config.h
 *
 * User configurations for the CVNP API. Only these values should
 * be changed by the end user. The defaults should be fine, but
 * strict memory requirements or high traffic may warrant changing
 * some parameters.
 *
 *  Created on: Mar 7, 2019
 *      Author: Duemmer
 */

#ifndef CVNP_CVNP_CONFIG_H_
#define CVNP_CVNP_CONFIG_H_

// Buffer size for multicast handlers
#define CVNP_MULTICAST_BUF_SIZE             4

// Buffer size for regular query handlers. As a rule of
// thumb this should be equal to the number of RX mailboxes,
// but this is not a requirement
#define CVNP_STD_QUERY_BUF_SIZE             16

/**
 * Size of the broadcast response handler buffer. This should be
 * large enough to handle every broadcast query that the device
 * wants to listen on.
 */
#define CVNP_BROADCAST_BUF_SIZE             16


/**
 * Size of the non-compliant frame handler buffer. This should be
 * large enough to handle every non-compliant frame that the device
 * wants to listen on.
 */
#define CVNP_NONCOMPLIANT_BUF_SIZE          16

#endif /* CVNP_CVNP_CONFIG_H_ */
