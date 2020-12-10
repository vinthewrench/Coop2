//
//  INA219.h
//
// this is a modified version of the Adafruit INA219 library
//  https://github.com/adafruit/Adafruit_INA219
// made to work on Spasrkfun Artemis
//

/*!
 * @file Adafruit_INA219.h
 *
 * This is a library for the Adafruit INA219 breakout board
 * ----> https://www.adafruit.com/product/904
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Bryan Siepert and Kevin "KTOWN" Townsend for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */


#ifndef INA219_h
#define INA219_h
 

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>


/** default I2C address **/
#define INA219_ADDRESS (0x40) // 1000000 (A0+A1=GND)


class INA219 {
 
	//Register Pointer Map
 enum INA219_Register : uint8_t
 {
	 /** config register address **/
	 INA219_REG_CONFIG = 0x00,

	 /** shunt voltage register **/
	 INA219_REG_SHUNTVOLTAGE  =  0x01,

	 /** bus voltage register **/
	 INA219_REG_BUSVOLTAGE  	=  0x02,

	 /** power register **/
	 INA219_REG_POWER = 0x03,

	 /** current register **/
	 INA219_REG_CURRENT= 0x04,

	 /** calibration register **/
	 INA219_REG_CALIBRATION = 0x05
 };
  
public:

	INA219();
	bool begin(uint8_t address = INA219_ADDRESS, TwoWire &wirePort = Wire);

	bool isConnected();

	void setCalibration_32V_2A();
	void setCalibration_32V_1A();
	void setCalibration_16V_400mA();
	
 	float getBusVoltage_V();
	float getShuntVoltage_mV();
	float getCurrent_mA();
	float getPower_mW();
	
	void powerSave(bool on);


private:
	TwoWire *_i2cPort;      //Generic connection to user's chosen I2C port
	uint8_t _deviceAddress; //I2C address of the button/switch

	
	uint32_t 	_calValue;

	// The following multipliers are used to convert raw current and power
	// values to mA and mW, taking into account the current config settings
	uint32_t 	_currentDivider_mA;
	float 		_powerMultiplier_mW;

	int16_t getBusVoltage_raw();
	int16_t getShuntVoltage_raw();
	int16_t getCurrent_raw();
	int16_t getPower_raw();
	
	bool	 writeDoubleRegister(INA219_Register reg, uint16_t data);
	uint16_t readDoubleRegister(INA219_Register reg);
};

#endif /* INA219_h */
