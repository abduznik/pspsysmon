export RELVER := 0.10

TARGET      := pspsysmon
BUILD_DIR   := build
SRC_DIR     := src

OBJS := $(SRC_DIR)/main.o \
        $(SRC_DIR)/display.o \
        $(SRC_DIR)/system.o \
        $(SRC_DIR)/config.o

INCDIR   := $(SRC_DIR)
CFLAGS   := -O2 -G0 -Wall \
            -Wno-implicit-function-declaration \
            -Wno-incompatible-pointer-types \
            -Wno-int-conversion \
            -Wno-int-to-pointer-cast
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS  := $(CFLAGS)

# PRX plugins must NOT link startup files
LDFLAGS := -nostartfiles

LIBDIR  :=
LIBS    := -lpsppower

BUILD_PRX       = 1
PRX_EXPORTS     := exports.exp

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

PSPSDK := $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

release: clean all
	@echo "==> Built $(TARGET).prx (v$(RELVER))"

pack: all
	mkdir -p $(BUILD_DIR)/temp/seplugins
	cp $(TARGET).prx $(BUILD_DIR)/temp/seplugins/
	echo "ms0:/seplugins/$(TARGET).prx 1" > $(BUILD_DIR)/temp/seplugins/game.txt
	echo "ms0:/seplugins/$(TARGET).prx 1" > $(BUILD_DIR)/temp/seplugins/vsh.txt
	cp README.md $(BUILD_DIR)/temp/
	cd $(BUILD_DIR)/temp && zip -r ../$(TARGET)-$(RELVER).zip *
	@echo "==> Packaged $(BUILD_DIR)/$(TARGET)-$(RELVER).zip"
