#
#  Makefile
#
#  (c) 2012-2013 by Markus Reschke
#  based on code from Markus Frejek and Karl-Heinz Kübbeler
#


#
# MCU settings
# - Change to fit your setup!
#

# avr-gcc: MCU model
# - ATmega168 or ATmega168P : atmega168
# - ATmega328 or ATmega328P : atmega328
MCU = atmega328
#MCU = atmega168

# MCU freqency:
# - 1MHz  : 1
# - 8MHz  : 8
# - 16MHz : 16
FREQ = 8

# oscillator type
# - internal RC oscillator      : RC
# - external full swing crystal : Crystal
# - external low power crystal  : LowPower
OSCILLATOR = Crystal

# oscillator start-up cycles
# - Crystal and LowPower could also be 1024 or 256 based on fuse settings
ifeq (${OSCILLATOR},RC)
  OSC_STARTUP = 6
endif
ifeq (${OSCILLATOR},Crystal)
  OSC_STARTUP = 16384
endif
ifeq (${OSCILLATOR},LowPower)
  OSC_STARTUP = 16384
endif


# avrdude: part number of MCU
# - ATmega168  : m168
# - ATmega168P : m168p
# - ATmega328  : m328
# - ATmega328P : m328p 
PARTNO = m328p

# avrdude: ISP programmer
#PROGRAMMER = buspirate
PROGRAMMER = avrispmkII

# avrdude: port of ISP programmer
#PORT = /dev/bus_pirate
PORT = usb


#
#  global settings
#

# project name
NAME = ComponentTester

# name and version based on directory name
DIST = $(notdir ${CURDIR})

# compiler flags
CC = avr-gcc
CPP = avr-g++
CFLAGS = -mmcu=${MCU} -Wall -I.
CFLAGS += -DF_CPU=${FREQ}000000UL
CFLAGS += -DOSC_STARTUP=${OSC_STARTUP}
CFLAGS += -gdwarf-2 -std=gnu99 -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

# linker flags
LDFLAGS = -mmcu=${MCU} -Wl,-Map=${NAME}.map

# hex file flags
HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

# header files
HEADERS = config.h common.h variables.h LCD.h functions.h

# objects
OBJECTS_C = main.o user.o pause.o adjust.o ADC.o probes.o LCD.o
OBJECTS_C += resistor.o cap.o semi.o inductor.o extras.o
OBJECTS_S = wait.o
OBJECTS = ${OBJECTS_C} ${OBJECTS_S}


#
#  build
#

all: ${NAME} ${NAME}.hex ${NAME}.eep ${NAME}.lss size


#
#  link
#

# link firmware
$(NAME): ${OBJECTS}
	 ${CC} ${LDFLAGS} ${OBJECTS} ${LIBDIRS} ${LIBS} -o ${NAME}

# create hex file of firmware
%.hex: ${NAME}
	avr-objcopy -O ihex ${HEX_FLASH_FLAGS}  $< $@

# create image for EEPROM
%.eep: ${NAME}
	-avr-objcopy ${HEX_EEPROM_FLAGS} -O ihex $< $@ || exit 0

# create dump of firmware
%.lss: ${NAME}
	avr-objdump -h -S $< > $@

# output size of firmware and stuff
size: ${NAME}
	@echo
	@avr-size -C --mcu=${MCU} ${NAME}


#
#  compile source files
#

# rule for all c-files
${OBJECTS_C}: %.o: %.c ${HEADERS} ${MAKEFILE_LIST}
	${CC} ${CFLAGS} -c ${@:.o=.c}

# rule for all S-files
${OBJECTS_S}: %.o: %.S ${HEADERS} ${MAKEFILE_LIST}
	${CC} ${CFLAGS} -c ${@:.o=.S}

# external dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)


#
#  extras
#

# upload firmware
upload: ${NAME} ${NAME}.hex ${NAME}.eep ${NAME}.lss size
	avrdude -c ${PROGRAMMER} -B 5.0 -p ${PARTNO} -P ${PORT} \
	  -U flash:w:./${NAME}.hex:a -U eeprom:w:./$(NAME).eep:a

# create distribution package
dist:
	rm -f *.tgz
	cd ..; tar -czf ${DIST}/${DIST}.tgz \
	  ${DIST}/*.h ${DIST}/*.c ${DIST}/*.S \
	  ${DIST}/Makefile ${DIST}/README ${DIST}/CHANGES

# clean up
clean:
	-rm -rf ${OBJECTS} ${NAME} dep/* *.tgz
	-rm -rf ${NAME}.hex ${NAME}.eep ${NAME}.lss ${NAME}.map


#
#  MCU fuses
#

# ATmega168 / ATmega168P 
ifeq (${MCU},atmega168)
  HFUSE = -U hfuse:w:0xdc:m
  ifeq (${FREQ},1)
    # internal RC oscillator (8MHz) and /8 clock divider
    LFUSE_RC = -U lfuse:w:0x62:m
    # external 8MHz full swing crystal and /8 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0x77:m
    # external 8MHz low power crystal and /8 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0x7f:m
  endif
  ifeq (${FREQ},8)
    # internal RC oscillator (8MHz) and /1 clock divider
    LFUSE_RC = -U lfuse:w:0xe2:m 
    # external 8MHz full swing crystal and /1 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0xf7:m
    # external 8MHz low power crystal and /1 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0xff:m
  endif
  ifeq (${FREQ},16)
    # internal RC oscillator (8MHz) not possible
    LFUSE_RC =
    # external 16MHz full swing crystal and /1 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0xf7:m
    # external 16MHz low power crystal and /1 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0xff:m
  endif
endif

# ATmega328 / ATmega328P
ifeq (${MCU},atmega328)
  HFUSE = -U hfuse:w:0xd9:m
  EFUSE = -U efuse:w:0xfc:m
  ifeq (${FREQ},1)
    # internal RC oscillator (8MHz) and /1 clock divider
    LFUSE_RC = -U lfuse:w:0x62:m
    # external 8MHz full swing crystal and /8 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0x77:m
    # external 8MHz low power crystal and /8 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0x7f:m
  endif
  ifeq (${FREQ},8)
    # internal RC oscillator (8MHz) and /1 clock divider
    LFUSE_RC = -U lfuse:w:0xe2:m
    # external 8MHz full swing crystal and /1 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0xf7:m
    # external 8MHz low power crystal and /1 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0xff:m
  endif
  ifeq (${FREQ},16)
    # internal RC oscillator (8MHz) not possible
    LFUSE_RC =
    # external 16MHz full swing crystal and /1 clock divider
    LFUSE_CRYSTAL = -U lfuse:w:0xf7:m
    # external 16MHz low power crystal and /1 clock divider
    LFUSE_LOWPOWER = -U lfuse:w:0xff:m
  endif
endif

# select LFUSE
ifeq (${OSCILLATOR},RC)
  LFUSE = ${LFUSE_RC}
endif
ifeq (${OSCILLATOR},Crystal)
  LFUSE = ${LFUSE_CRYSTAL}
endif
ifeq (${OSCILLATOR},LowPower)
  LFUSE = ${LFUSE_LOWPOWER}
endif

# check fuses
FUSES =
ifneq ($(strip ${LFUSE}),)
  ifneq ($(strip ${HFUSE}),)
    FUSES = ${LFUSE} ${HFUSE} ${EFUSE}
  endif
endif

# set fuses
fuses:
  ifeq ($(strip ${FUSES}),)
	@echo Invalid fuse settings!
  else
	avrdude -c ${PROGRAMMER} -B 10.0 -p ${PARTNO} -P ${PORT} ${FUSES}
  endif
