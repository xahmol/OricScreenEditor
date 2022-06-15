# Oric Screen Editor
# Screen editor for the Oric Atmos
# Written in 2022 by Xander Mol
# Based on VDC Screen Editor for the C128
# 
# https://github.com/xahmol/OricScreenEditor
# https://www.idreamtin8bits.com/

# For full credits: see src/main.c

# Thanks to Iss for pointers for this Makefile:
# https://forum.defence-force.org/viewtopic.php?p=25411#p25411

## Paths
# Path tp OSDK install. Edit for local dir
# Installation instructions for Linux: #https://forum.defence-force.org/viewtopic.php?p=25396#p25396
OSDK := /home/xahmol/OSDK-build/pc/tools/osdk/main/osdk-linux/bin/
# Path tp HxC Floppy Emulator command line tool: edit for local dir
# SVN repository: svn checkout https://svn.code.sf.net/p/hxcfloppyemu/code/ hxcfloppyemu-code
# Build with  HxCFloppyEmulator/build/make 
HXCFE := /home/xahmol/hxcfloppyemu-cod/HxCFloppyEmulator/HxCFloppyEmulator_cmdline/trunk/build/
# Emulator path: edit for location of emulator to use
EMUL_DIR := /home/xahmol/oricutron/
# Makefile full path
mkfile_path := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

## Project name
PROJECT      := OSE
PROJECT_DIR  := $(shell pwd)
PROGRAM      := BUILD/$(PROJECT).bin
DISK_DSK     := BUILD/$(PROJECT).dsk
DISK_HFE     := BUILD/$(PROJECT).hfe

# Arguments for emulator
# # # Use MACH to override the default and run as:
# # # make run-tap MACH=-ma for Atmos (default)
# # # make run-tap MACH=-m1 for Oric-1
# # # make run-tap MACH=-mo16k for Oric-1 / 16K
# # # make run-tap MACH=-mp for Pravetz-8D
# # # make run-tap MACH=-mt for Telestrat
MACH := -ma
EMU                     := ./oricutron
EMUDIR                  := $(EMUL_DIR)
EMUARG                  := $(MACH)
# # # (un)comment, add, remove more Oricutron cmd-line options
EMUARG                  += --serial none
EMUARG                  += --vsynchack off
EMUARG                  += --turbotape on

## C Sources and library objects to use
SOURCES = src/main.c src/oric_core.c
LIBOBJECTS = src/oric_core_assembly.s

## Compiler and linker flags
CC65_TARGET = atmos
CC = cl65
CFLAGS  = -t $(CC65_TARGET) --create-dep $(<:.c=.d) -Oirs -I include
LDFLAGS = -t $(CC65_TARGET) -C oricse_cc65_config.cfg -m $(PROJECT).map

########################################

.SUFFIXES:
.PHONY: all clean run run-debug
all: $(PROGRAM) $(DISK_DSK)

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d) $(SOURCESUPD:.c=.d)
endif

# Compile C sources
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

# Link compiled objects 
$(PROGRAM): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBOBJECTS)

# Build disk
$(DISK_DSK): $(PROGRAM) $(TESTPROG_BIN)
	$(OSDK)header $(PROGRAM) BUILD/$(PROJECT).tap 0x0501
	$(OSDK)tap2dsk -iCLS:$(PROJECT) -c20:3 -n$(PROJECT) BUILD/$(PROJECT).tap OSEHS1.tap OSEHS2.tap OSEHS3.tap OSEHS4.tap OSETSC.tap $(DISK_DSK)
	$(OSDK)old2mfm $(DISK_DSK)
	cd $(HXCFE); ./hxcfe -finput:"$(mkfile_path)/$(DISK_DSK)" -foutput:"$(mkfile_path)/$(DISK_HFE)" -conv:HXC_HFE

# Clean old builds and objects
clean:
	$(RM) $(SOURCES:.c=.o) $(SOURCES:.c=.d) $(PROGRAM) $(PROGRAM).map $(PROGRAM).brk $(PROGRAM).sym
	cd BUILD; $(RM) *.*

# Execute in emulator: use make run
run: $(DISK_DSK)
	cd $(EMUDIR); $(EMU) $(EMUARG) "$(PROJECT_DIR)/$(DISK_DSK)"