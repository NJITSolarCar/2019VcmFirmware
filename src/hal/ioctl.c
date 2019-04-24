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
#include <driverlib/interrupt.h>

#include "ioctl.h"
#include "resource.h"
#include "../fault.h"
#include "../util.h"
#include "../device/relay.h"


/**
 * Called when the main system watchdog expires. Usually as a result
 * of a trap, crash, etc. Immediately cut the outputs, disable interrupts,
 * and assert a WDT_FAIL fault, which should send a final message over CAN
 * that the VCM has crashed.
 */
void _ioctl_watchdog_expire() {
	IntMasterDisable();
	relay_setAll(false);

	tFaultData dat;
	dat.ui64 = 0;
	fault_assert(FAULT_VCM_WDT_FAIL, dat);

	for(;;);
}


void ioctl_reset() {
	// clock up to 80 MHz
    SysCtlClockSet(
            SYSCTL_OSC_MAIN |
			IOCTL_MCU_XTAL |
			SYSCTL_USE_PLL |
			IOCTL_MCU_PLL_DIV
    );

    uint32_t clk = SysCtlClockGet();

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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

    // Turn on the FPU
    FPULazyStackingEnable();
    FPUEnable();

    // Configure and start the clock timer
    TimerConfigure(SYS_US_TIMER_BASE, TIMER_CFG_ONE_SHOT_UP);
    TimerLoadSet64(SYS_US_TIMER_BASE, 0);
    TimerPrescaleSet(SYS_US_TIMER_BASE, TIMER_A, 80);
    TimerEnable(SYS_US_TIMER_BASE, TIMER_A);

    // Configure (but don't start) the watchdog with Non-maskable interrupt,
    // and halt during debug events
    WatchdogIntTypeSet(SYS_WATCHDOG, WATCHDOG_INT_TYPE_NMI);
    WatchdogIntRegister(SYS_WATCHDOG, _ioctl_watchdog_expire);
    WatchdogStallEnable(SYS_WATCHDOG);
    WatchdogResetDisable(SYS_WATCHDOG);
    ioctl_loadWatchdog();
}




void ioctl_loadWatchdog() {
	WatchdogReloadSet(SYS_WATCHDOG, IOCTL_WDT_LOAD);
}



