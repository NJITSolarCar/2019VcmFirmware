/*
 * relay.h
 *
 *  Created on: Apr 19, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_RELAY_H_
#define SRC_DEVICE_RELAY_H_

#include <stdbool.h>

/**
 * Initialize relay system
 */
void relay_init();


/**
 * Relay controls
 */
void relay_setBattPlus(bool on);
void relay_setBattMinus(bool on);
void relay_setDischarge(bool on);
void relay_setCharge(bool on);
void relay_setSolar(bool on);

/**
 * Relay statuses
 */
bool relay_getBattPlus();
bool relay_getBattMinus();
bool relay_getDischarge();
bool relay_getCharge();
bool relay_getSolar();

// Utility function to set all relays
void relay_setAll(bool on);


/**
 * Master enable for relays. Acts as a mask with
 * the values passed to relay_set*(). If enable is set
 * to true, the relays will operate normally. If not, relays can still
 * be set, but will not turn on until true is passed.
 */
void relay_enable(bool enable);

/**
 * Sets all relays according to the faults set at the
 * time of calling
 *
 * TODO: A potential race condition exists with this method and the
 * fault system. If the fault status is read, and an interrupt that asserts
 * a fault blocks the relay updates, it is possible for a relay to stay enabled
 * when it should be cleared. As such, this function should be used sparingly, if
 * at all.
 */
void relay_setFromFaults();


/**
 * Overrides the current relay state to force the outputs on
 */
void relay_override(bool enable);


#endif /* SRC_DEVICE_RELAY_H_ */
