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






static void vcm_BmsVoltWarnHandler(tFaultData dat) {
	indicator_setPattern(LED_STAT_VOLT_WARN);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsPackShortFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_PACK_SHORT);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsCommFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_COMM);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsOverCurrentChgFaultHandler(tFaultData dat) {
	relay_setSolar(false);
	relay_setCharge(false);
	indicator_setPattern(LED_STAT_OVER_CHG_I);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsOverCurrentDischgFaultHandler(tFaultData dat) {
	relay_setDischarge(false);
	indicator_setPattern(LED_STAT_OVER_DISCHG_I);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsOverVoltFaultHandler(tFaultData dat) {
	relay_setSolar(false);
	relay_setCharge(false);
	indicator_setPattern(LED_STAT_OVER_VOLT);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsUnderVoltFaultHandler(tFaultData dat) {
	relay_setDischarge(false);
	indicator_setPattern(LED_STAT_UNDER_VOLT);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_BmsGenFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_GEN_FAULT);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_TempFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_TEMP_FAULT);
	vcm_defaultFaultAssertAction(dat);
}




static void vcm_TempWarnHandler(tFaultData dat) {
	indicator_setPattern(LED_STAT_TEMP_WARN);
	vcm_defaultFaultAssertAction(dat);
}



/**
 * NOTE: For all MPPT faults except FAULT_MPPT_NO_BATT, which needs to
 * be addressed separately, as it can be thrown during otherwise normal
 * operation (charge relay opened)
 */
static void vcm_mpptGenFaultHandler(tFaultData dat) {
	relay_setSolar(false);
	indicator_setPattern(LED_STAT_MPPT_FAULT);
	vcm_defaultFaultAssertAction(dat);
}



static void vcm_mpptNoBattFaultHandler(tFaultData dat) {
	indicator_setPattern(LED_STAT_MPPT_FAULT);
	vcm_defaultFaultAssertAction(dat);
}



static void vcm_mpptCommFaultHandler(tFaultData dat) {
	relay_setSolar(false);
	indicator_setPattern(LED_STAT_COMM);
	vcm_defaultFaultAssertAction(dat);
}



static void vcm_kblGenFaultHandler(tFaultData dat) {
	relay_setDischarge(false);
	indicator_setPattern(LED_STAT_KBL_FAULT);
	vcm_defaultFaultAssertAction(dat);
}



static void vcm_kblCommFaultHandler(tFaultData dat) {
	relay_setDischarge(false);
	indicator_setPattern(LED_STAT_COMM);
	vcm_defaultFaultAssertAction(dat);
}



/**
 * For FAULT_CVNP_INTERNAL, FAULT_VCM_COMM, FAULT_GEN_AUX_OVER_DISCHARGE,
 * and FAULT_GEN_ESTOP
 */
static void vcm_vcmGenFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_GEN_FAULT);
	vcm_defaultFaultAssertAction(dat);
}



static void vcm_vcmCrashHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_VCM_CRASH);
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
