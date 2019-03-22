CC=gcc
CFLAGS=-I. -Wall -Wextra -Wno-unused-parameter -DCVNP_DEBUG_ENABLE
INC = cvnp.h cvnp_hal.h cvnp_config.h

cvnp: cvnp.o cvnp_test.o
	$(CC) -o cvnp cvnp.o cvnp_test.o
	rm -rf *.o *.gch
	
cvnp.o: cvnp.c $(INC)
	$(CC) -c cvnp.c $(CFLAGS)
	
cvnp_test.o: cvnp_test.c $(INC)
	$(CC) -c cvnp_test.c $(CFLAGS)
	
clean:
	rm -rf *.o *.gch cvnp