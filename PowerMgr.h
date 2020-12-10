//
//  PowerMgr.h
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

#ifndef PowerMgr_h
#define PowerMgr_h

#include "INA219.h"
#include <stdio.h>

class PowerMgr
{

public:
	
	PowerMgr();
	
	bool 	begin(uint8_t ACpowerPin);
	bool 	hasSensor();

	bool 	hasACpower();

	float	voltage();
//	float	power_mW();
	float current_mA();
	
//	float 	average_power_mW();
	float 	average_current_mA();

	void 	loop();  // for calculating running average

private:

	bool 		_hasACSensor;
	uint8_t 	_ACpowerPin;
	
	bool 		_hasIna219;
	INA219		_ina219;
 
	float		_curMvAvg;
	float 		_curMvAvgSum;
//
//	float		_pwMvAvg;
//	float 		_pwMvAvgSum;
const byte 	_AvgCount = 100;
	
};


#endif /* PowerMgr_h */
