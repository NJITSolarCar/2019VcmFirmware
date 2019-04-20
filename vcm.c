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

// Common C includes
#include <stdbool.h>
#include <stdint.h>

// General purpose files
#include "src/fault.h"
#include "src/interrupt.h"
#include "src/util.h"

// Specific modules / devices
#include "src/device/bms.h"
#include "src/device/mppt.h"
#include "src/device/kbl.h"
#include "src/device/ina225.h"
#include "src/device/indicator.h"
#include "src/device/vcm_io.h"
#include "src/device/relay.h"
#include "src/device/thermo.h"

// Hardware access
#include "src/hal/adc.h"
#include "src/hal/gpio.h"
#include "src/hal/ioctl.h"
#include "src/hal/resource.h"

// CVNP
#include "src/cvnp/cvnp.h"

// Driverlib includes
#include <driverlib/systick.h>

#define VCM_CVNP_MYCLASS			1
#define VCM_CVNP_MYINST				1

#define VCM_SYSTICK_LOAD			(UTIL_CYCLE_PER_MS * 10) // 10ms tick rate


/////////////////////////////// FAULT BINDINGS ////////////////////////////////

/**
 * Base action for all faults: report it over CVNP. This will send a broadcast frame
 * with information about this fault.
 */
static void vcm_defaultFaultAssertAction(tFaultData dat) {
	// TODO: implement broadcast transmission via CVNP
}


/**
 * Handler called when FAULT_BMS_PACK_SHORT is asserted.
 * Disable all relays, set the indicator state, and do default
 * action
 */
static void vcm_BmsPackShortHandler(tFaultData dat) {
	relay_setAll(false);
	vcm_defaultFaultAssertAction(dat);
}



/**
 * Handler called when FAULT_BMS_COMM is asserted.
 * Disable all relays, set the indicator state, and do default
 * action
 */
static void vcm_BmsCommHandler(tFaultData dat) {
	relay_setAll(false);
	vcm_defaultFaultAssertAction(dat);
}


/**
 * Handler called when FAULT_BMS_OVER_CURRENT_CHG is asserted.
 * Disable charge / solar relays, set the indicator state, and do default
 * action
 */
static void vcm_BmsOverCurrentChg(tFaultData dat) {
	relay_setSolar(false);
	relay_setCharge(false);
	vcm_defaultFaultAssertAction(dat);
}



/**
 * Bind all of the fault handlers for the VCM
 */
static void vcm_bindFaultHandlers() {

}



/////////////////////////////// MAIN ALGORITHM ////////////////////////////////
static void vcm_tick() {
	bms_tick();
	kbl_tick();
	mppt_tick();
	cvnp_tick();
}




int main(void)
{
	// Startup sequence
	ioctl_reset();
	gpio_init();
	adc_init();

	cvnp_start(VCM_CVNP_MYCLASS, VCM_CVNP_MYINST);

	// Module startup
	fault_init();
	bms_init();
	ina_init();
	indicator_init();
	kbl_init();
	mppt_init();
	relay_init();
	thermo_init();
	vcmio_init();

	// Systick startup
	SysTickPeriodSet(VCM_SYSTICK_LOAD);
	SysTickIntRegister(vcm_tick);
	SysTickIntEnable();
	SysTickEnable();


	return 0;
}
