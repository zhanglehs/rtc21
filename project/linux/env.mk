#CC = gcc
CC = g++
CFLAGS = -g -m64 -Wall -fno-strict-aliasing -fomit-frame-pointer -Wfloat-equal -rdynamic -Wformat=2  -Wno-cast-qual -pipe
SOURCE_DIR = ../../src/
CINCS = -I $(SOURCE_DIR)
LIBS = -lrt 
DEFS = -DHAVE_SCHED_GETAFFINITY

%.o:%.cc
	        $(CC) -c $(CFLAGS) $(CINCS) $(DEFS) $^ -o $@

%.o:%.c
	        $(CC) -c $(CFLAGS) $(CINCS) $(DEFS) $^ -o $@

%.o:%.cpp
	        $(CC) -c $(CFLAGS) $(CINCS) $(DEFS) $^ -o $@
