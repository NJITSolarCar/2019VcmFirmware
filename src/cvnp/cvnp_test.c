/*
 * cvnp_test.c
 *
 *	Unit testing for the core, platform independent CVNP library.
 *	This is intended to be run on a fully-featured C system, not
 *	on the embedded target. This will create a child process for simulating
 *	the CAN bus, which will send and receive various messages in order
 *	to test the system
 *
 *  Created on: Mar 22, 2019
 *      Author: Duemmer
 */


/**
 * Debug enable. If this constant is enabled, the CVNP debug routines
 * will be included in the build. This includes a cvnp_hal implementation,
 * test routines, and a main() function to run it. This is designed to
 * run on a test bench setup, so printf, fprintf, fork, etc. are used.
 * This constant will be set by cvnp_debug.make
 */
#ifdef CVNP_DEBUG_ENABLE

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "cvnp_config.h"
#include "cvnp_hal.h"
#include "cvnp.h"

#define MY_INST 1
#define MY_CLASS 1

char buf[1024]; // Read buffer

// Pipes for communication between the CAN child and parent
int p1[2], p2[2];

// cvnp_hal implementations. Stubs for now.
bool cvnpHal_init() {

}

void cvnpHal_sendFrame(tCanFrame frame) {

}

uint32_t cvnpHal_now() {

}

void cvnpHal_handleError(uint32_t errNum) {

}

void cvnpHal_resetSystem() {

}




/**
 * Main test routine for the CVNP main code
 */
void test_parent() {
	// Setup pipes
	bool didStart = cvnp_start(MY_CLASS, MY_INST);
}


/**
 * Main test routine for the simulated CAN client. This will loop and receive
 * input frames via the pipes, as well as send its own frames periodically
 *
 */
void test_child() {

}



int main() {
	pid_t proc;

	// open pipes
	if (pipe(p1)==-1)
	{
		fprintf(stderr, "Pipe Failed" );
		return 1;
	}
	if (pipe(p2)==-1)
	{
		fprintf(stderr, "Pipe Failed" );
		return 1;
	}

	// Fork
	proc = fork();
	if(p < 0)
	{
		fprintf(stderr, "fork Failed" );
		return 1;
	} else if(p > 0) // in the parent proc. Main CVNP control goes here
		test_parent();
	else // in the child proc. Simulated bus goes here
		test_child();
}










#endif
