OBJS	= src/hpsdr_debug.o  src/hpsdr_functions.o  src/hpsdr_newprotocol.o  src/hpsdr_sim.o
SOURCE	= src/hpsdr_debug.c  src/hpsdr_functions.c  src/hpsdr_newprotocol.c  src/hpsdr_sim.c
HEADER	= src/hpsdr_debug.h  src/hpsdr_definitions.h  src/hpsdr_functions.h  src/hpsdr_sim.h
OUT	= hpsdr_sim
CC	?= gcc

DEBUG ?= 0

BASE_CFLAGS = -Wall -Wextra -pipe
ifeq ($(DEBUG),1)
CFLAGS = $(BASE_CFLAGS) -O0 -g
else
CFLAGS = $(BASE_CFLAGS) -O3 -DNDEBUG
endif

FLAGS	 = $(CFLAGS) -c
LFLAGS	 = -lm -lrt -lpthread

.PHONY: all clean release debug

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(OUT) $(LFLAGS)

release:
	$(MAKE) clean
	$(MAKE) DEBUG=0 all

debug:
	$(MAKE) clean
	$(MAKE) DEBUG=1 all

hpsdrsim.o: src/hpsdr_sim.c
	$(CC) $(FLAGS) hpsdr_sim.c 

newhpsdrsim.o: src/hpsdr_newprotocol.c
	$(CC) $(FLAGS) hpsdr_newprotocol.c 

hpsdr_debug.o: src/hpsdr_debug.c
	$(CC) $(FLAGS) hpsdr_debug.c

hpsdr_functions.o: src/hpsdr_functions.c
	$(CC) $(FLAGS) hpsdr_functions.c

clean:
	rm -f $(OBJS) $(OUT)





