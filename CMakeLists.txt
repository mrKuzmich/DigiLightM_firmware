###########################################################
# cmake directives file was received from
# https://www.kuon.ch/post/2018-07-11-avr-cmake/
###########################################################

# I target a recent cmake, it shouldn't be a problem on a dev machine
cmake_minimum_required(VERSION 3.11)

# Project name
project("DigiLight-M" C ASM)

# Product filename
set(PRODUCT_NAME ${PROJECT_NAME})

## AVR Chip Configuration
# 16Mhz, this should match the crystal on your board,
set(F_CPU 16000000UL)
set(MCU atmega32)

# Avrdude programmer configuration
set(AVRDUDE_PATH /home/dmitry/Projects/avrdude/RELEASE_6_3)
set(AVRDUDE ${AVRDUDE_PATH}/avrdude)
set(AVRDUDE_CONF ${AVRDUDE_PATH}/avrdude.conf)
set(AVRDUDE_PROG apu2)

# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega32&LOW=BF&HIGH=C7&LOCKBIT=FF
#set(E_FUSE 0xfd)
set(H_FUSE 0xC7)
set(L_FUSE 0xBF)
#set(LOCK_BIT 0xff)

# Use AVR GCC toolchain
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_ASM_COMPILER avr-gcc)

# Pass defines to compiler
add_definitions(
    -DF_CPU=${F_CPU}
)
# mmcu MUST be passed to bot the compiler and linker, this handle the linker
set(CMAKE_EXE_LINKER_FLAGS -mmcu=${MCU})

add_compile_options(
    -mmcu=${MCU} # MCU
    -std=gnu99 # C99 standard
    -Os # optimize
    -Wall # enable warnings
    -Wno-main
    -Wundef
#    -pedantic
    -Wstrict-prototypes
#    -Werror
    -Wfatal-errors
    -Wl,--relax,--gc-sections
    -g
    -gdwarf-2
    -funsigned-char # a few optimizations
    -funsigned-bitfields
    -fpack-struct
    -fshort-enums
    -ffunction-sections
    -fdata-sections
    -fno-split-wide-types
    -fno-tree-scev-cprop
)

file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.S") # Load all files in src/** folders
list(FILTER SRC_FILES EXCLUDE REGEX ".*sd_card_example.c$") # Exclude sd_card_example.c from src files
include_directories("src/" "src/PFFS") # Header (*.h) files directories

# Create one target
add_executable(${PRODUCT_NAME} ${SRC_FILES})

# Rename the output to .elf as we will create multiple files
set_target_properties(${PRODUCT_NAME} PROPERTIES OUTPUT_NAME ${PRODUCT_NAME}.elf)

# Strip binary for upload
add_custom_target(strip ALL avr-strip ${PRODUCT_NAME}.elf DEPENDS ${PRODUCT_NAME})

# Transform binary into hex file, we ignore the eeprom segments in the step
add_custom_target(hex ALL avr-objcopy -R .eeprom -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.hex DEPENDS strip)

# Transform binary into hex file, this is the eeprom part (empty if you don't
# use eeprom static variables)
add_custom_target(eeprom avr-objcopy -j .eeprom  --set-section-flags=.eeprom="alloc,load"  --change-section-lma .eeprom=0 -O ihex ${PRODUCT_NAME}.elf ${PRODUCT_NAME}.eep DEPENDS strip)

# Upload the firmware with avrdude
add_custom_target(upload ${AVRDUDE} -C ${AVRDUDE_CONF} -c ${AVRDUDE_PROG} -p ${MCU} -U flash:w:${PRODUCT_NAME}.hex DEPENDS hex)

# Upload the eeprom with avrdude
add_custom_target(upload_eeprom ${AVRDUDE} -C ${AVRDUDE_CONF} -c ${AVRDUDE_PROG} -p ${MCU} -U eeprom:w:${PRODUCT_NAME}.eep DEPENDS eeprom)

# Burn fuses
add_custom_target(fuses ${AVRDUDE} -C ${AVRDUDE_CONF} -c ${AVRDUDE_PROG} -p ${MCU} -U lfuse:w:${L_FUSE}:m -U hfuse:w:${H_FUSE}:m)

# Avrdude test read
add_custom_target(test ${AVRDUDE} -C ${AVRDUDE_CONF} -c ${AVRDUDE_PROG} -p ${MCU})

# Clean extra files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${PRODUCT_NAME}.hex;${PRODUCT_NAME}.eeprom;${PRODUCT_NAME}.lst")
