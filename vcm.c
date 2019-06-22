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
#include "src/device/debug_led.h"

// Hardware access
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



static void vcm_mpptUserLockoutFaultHandler(tFaultData dat) {
	relay_setSolar(false);
	indicator_setPattern(LED_STAT_MPPT_LOCK);
}


static void vcm_vcmThermFaultHandler(tFaultData dat) {
	relay_setAll(false);
	indicator_setPattern(LED_STAT_THERM_WIRING);
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


static void vcm_defaultDeassert() {
	// Do nothing for now
}


/**
 * Bind all of the fault handlers for the VCM
 */
static void vcm_bindFaultHandlers() {
	fault_regHook(FAULT_BMS_CELL_OVER_VOLT_WARN, vcm_BmsVoltWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_CELL_UNDER_VOLT_WARN, vcm_BmsVoltWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_PACK_OVER_VOLT_WARN, vcm_BmsVoltWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_PACK_UNDER_VOLT_WARN, vcm_BmsVoltWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_PACK_SHORT, vcm_BmsPackShortFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_COMM, vcm_BmsCommFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_OVER_CURRENT_CHG, vcm_BmsOverCurrentChgFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_OVER_CURRENT_DISCHG, vcm_BmsOverCurrentDischgFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_CELL_OVER_VOLT, vcm_BmsOverVoltFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_CELL_UNDER_VOLT, vcm_BmsUnderVoltFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_PACK_OVER_VOLT, vcm_BmsOverVoltFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_PACK_UNDER_VOLT, vcm_BmsUnderVoltFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_LOW_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_HIGH_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_IMBALANCE, vcm_BmsGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_BMS_GENERAL, vcm_BmsGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_TEMP_WARN, vcm_TempWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_BATT_CHARGED, vcm_mpptGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_NO_BATT, vcm_mpptNoBattFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_LOW_SOLAR_VOLTS, vcm_mpptGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_COMM, vcm_mpptCommFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_TELE_LORA, vcm_defaultFaultAssertAction, vcm_defaultDeassert);
	fault_regHook(FAULT_TELE_PI, vcm_defaultFaultAssertAction, vcm_defaultDeassert);
	fault_regHook(FAULT_TELE_COMM, vcm_defaultFaultAssertAction, vcm_defaultDeassert);
	fault_regHook(FAULT_MOTOR_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MOTOR_GEN, vcm_kblGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MOTOR_COMM, vcm_kblCommFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_GEN_ESTOP, vcm_vcmCrashHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_GEN_AUX_OVER_DISCHARGE, vcm_defaultFaultAssertAction, vcm_defaultDeassert);
	fault_regHook(FAULT_CVNP_INTERNAL, vcm_defaultFaultAssertAction, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_COMM, vcm_vcmGenFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_WDT_FAIL, vcm_vcmCrashHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_THERMISTOR, vcm_vcmThermFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_HIGH_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_TEMP_WARN, vcm_TempWarnHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_VCM_LOW_TEMP, vcm_TempFaultHandler, vcm_defaultDeassert);
	fault_regHook(FAULT_MPPT_USER_LOCKOUT, vcm_mpptUserLockoutFaultHandler, vcm_defaultDeassert);

}



/////////////////////////////// MAIN ALGORITHM ////////////////////////////////
static void vcm_tick() {
	float g_thermoTemp[3]; // Thermistor temperature readings

	// calc results
	thermo_getTemp(g_thermoTemp);

	gpio_tick();
	bms_tick();
	kbl_tick();
//	mppt_tick();
	cvnp_tick();

	// Always sample the thermistor and report
	thermo_doSample();
}




int main(void)
{
	// Fault handler startup
	fault_init();
	vcm_bindFaultHandlers();

	// Startup sequence
	ioctl_reset();

	// Start this first for debugging purposes
	debugLed_init();
	debugLed_setSolid(true);

	gpio_init();

	cvnp_start(VCM_CVNP_MYCLASS, VCM_CVNP_MYINST);

//	GPIOPinTypeGPIOInput(CAN_RX_PORT, CAN_RX_PIN);
//	GPIOPinTypeGPIOInput(CAN_TX_PORT, CAN_TX_PIN);

	// Module startup
	indicator_init();
	bms_init();
	ina_init();
	kbl_init();
//	mppt_init();
	relay_init();
	thermo_init();
	vcmio_init();

	// relay_enable(true);
	relay_enable(false);
	relay_setAll(true);
	indicator_setPattern(LED_STAT_NOFLT_DISBL);



	// Systick startup
	SysTickPeriodSet(VCM_SYSTICK_LOAD);
	SysTickIntRegister(vcm_tick);
	SysTickIntEnable();
	SysTickEnable();

	// Start blinking
	debugLed_setBlink();

	// Loop forever. Everything will be done via external event triggers
	// or the systick interrupt
	for(;;);
}
