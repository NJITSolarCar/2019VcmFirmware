/*
 * ioctl.c
 *
 *  Created on: Apr 19, 2019
 *      Author: Duemmer
 */


#include <stdint.h>
#include <stdbool.h>

#include <driverlib/sysctl.h>
#include <driverlib/watchdog.h>
#include <driverlib/fpu.h>

#include "ioctl.h"
#include "../util.h"
#include "resource.h"


/**
 * Called when the main system watchdog expires. Usually as a result
 * of a trap, crash, etc. Immediately cut the outputs, disable interrupts,
 * and trap here to preserve the system state.
 */
void _ioctl_watchdog_expire() {

}


void ioctl_reset() {
	// clock up to 80 MHz
    SysCtlClockSet(
            SYSCTL_OSC_MAIN |
			IOCTL_MCU_XTAL |
			IOCTL_MCU_VCO |
			IOCTL_MCU_PLL_DIV
    );

    // Enable peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

    // Turn on the FPU
    FPULazyStackingEnable();
    FPUEnable();

    // Configure and start the clock timer
    TimerConfigure(SYS_US_TIMER_BASE, TIMER_CFG_ONE_SHOT_UP);
    TimerLoadSet64(SYS_US_TIMER_BASE, 0);
    TimerPrescaleSet(SYS_US_TIMER_BASE, TIMER_A, 80);
    TimerEnable(SYS_US_TIMER_BASE, TIMER_A);

    // Configure and start the watchdog with Non-maskable interrupt,
    // and halt during debug events
    WatchdogIntTypeSet(SYS_WATCHDOG, WATCHDOG_INT_TYPE_NMI);
    WatchdogIntRegister(SYS_WATCHDOG, _ioctl_watchdog_expire);
    WatchdogStallEnable(SYS_WATCHDOG);
    WatchdogResetDisable(SYS_WATCHDOG);
    ioctl_loadWatchdog();
    WatchdogEnable(SYS_WATCHDOG);
}




void ioctl_loadWatchdog() {
	WatchdogReloadSet(SYS_WATCHDOG, IOCTL_WDT_LOAD);
}



