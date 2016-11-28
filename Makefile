
# Check if PATH is configured correctly
prefix :=$(shell xeno-config --prefix)

ifeq ($(prefix),)
$(error Please add <xeno-install-path>/bin to your PATH variable)
endif

# Get compiler and linker flags
CC := $(shell xeno-config --cc)
CFLAGS := $(shell xeno-config --skin=native --cflags)
LDFLAGS := -lnative $(shell xeno-config --skin=native --ldflags)

# Compile application
pong: pong.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS) -Xlinker -rpath -Xlinker /usr/xenomai/lib `sdl-config --cflags --libs`

clean: 
	rm pong
