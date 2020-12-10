# Arduino Library base folder and example structure
APP_BASE = .
APPNAME ?= WifiArtTest

# Arduino CLI executable name and directory location
ARDUINO_CLI = arduino-cli
ARDUINO_CLI_DIR = .

# Arduino CLI Board type
#BOARD_TYPE ?= arduino:avr:uno
BOARD_TYPE ?= SparkFun:apollo3:artemis
#BOARD_TYPE ?= arduino:avr:mega

# Default port to upload to
SERIAL_PORT ?= /dev/cu.usbserial-1420

 # Build path -- used to store built binary and object files
BUILD_DIR= build
BUILD_PATH= $(APP_BASE)/$(BUILD_DIR)
 
ifneq ($(V), 0)
    VERBOSE=-v
endif

.PHONY: all compile flash clean

all: compile

compile:
	$(ARDUINO_CLI) compile --fqbn SparkFun:apollo3:artemis $(APP_BASE)/$(APPNAME)

flash:
	artemis_svl /dev/cu.usbserial-1410 -f $(BUILD_PATH)/SparkFun.apollo3.artemis/WifiArtTest -b 460800 -v
 
clean:
	@rm -Rf  $(BUILD_PATH)


 
