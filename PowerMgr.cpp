//
//  PowerMgr.cpp
//  Coop2
//

/*****************************************************************************
 
MIT License

 Copyright (c) 2020 Vinnie Moscaritolo
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

#include "PowerMgr.h"
#include "CommonIncludes.h"

PowerMgr::PowerMgr() {
	
}
 
bool PowerMgr::begin(uint8_t ACpowerPin){
	
	_hasACSensor 	= (ACpowerPin != 0);
	if(_hasACSensor){
		
		pinMode(ACpowerPin, INPUT);
		pinMode(ACpowerPin, INPUT_PULLUP);
		_ACpowerPin 	= ACpowerPin;
	}

	_hasIna219 = _ina219.begin();
	if(_hasIna219){
		_ina219.setCalibration_16V_400mA();
	
		
		// setup initial average readings
		// Pre-load MMA
		for (int i=0 ; i < _AvgCount; i++){
//			_pwMvAvgSum = _pwMvAvgSum + _ina219.getPower_mW();
			_curMvAvgSum = _curMvAvgSum + _ina219.getCurrent_mA();
		}
			 
		  // Calculate inital average
//		_pwMvAvg = _pwMvAvgSum / _AvgCount;
		_curMvAvg = _curMvAvgSum / _AvgCount;
		 
//		_ina219.setCalibration_32V_2A();

	}
	
	return _hasIna219;
}

// for calculating running average
void 	PowerMgr::loop(){
//
//	// Remove previous movingAverage from the sum
//	_pwMvAvgSum = _pwMvAvgSum - _pwMvAvg;
//
//	 // Replace it with the current sample
//	_pwMvAvgSum = _pwMvAvgSum + _ina219.getPower_mW();
//
//	// Recalculate movingAverage
//	_pwMvAvg = _pwMvAvgSum / _AvgCount;

	
	// Remove previous movingAverage from the sum
	_curMvAvgSum = _curMvAvgSum - _curMvAvg;
	
	 // Replace it with the current sample
	_curMvAvgSum = _curMvAvgSum + _ina219.getCurrent_mA();

	// Recalculate movingAverage
	_curMvAvg = _curMvAvgSum / _AvgCount;
	
}


bool PowerMgr::hasACpower(){
	
	if(_hasACSensor)
		return (digitalRead(_ACpowerPin) == LOW);

	return true;
}

bool PowerMgr::hasSensor(){
	return _hasIna219;
}

float PowerMgr::voltage() {
	
	return _ina219.getBusVoltage_V();
}
//
//float PowerMgr::power_mW() {
//
//	return _ina219.getPower_mW();
//}
//

float PowerMgr::current_mA(){
	
	return _ina219.getCurrent_mA();
}
//
//float PowerMgr::average_power_mW() {
//	
//	return  _pwMvAvg;
//}


float 	PowerMgr::average_current_mA(){
	return _curMvAvg;
}
