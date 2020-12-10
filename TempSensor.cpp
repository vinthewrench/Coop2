//
//  TempSensor.cpp
//  WifiArtTest
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

#include "TempSensor.h"


bool TempSensor::setup(){
	
	hasSensor = this->begin();
	
	if(hasSensor){
		this->setAlertMode(0); // Comparator Mode.
		// set the Conversion Rate (how quickly the sensor gets a new reading)
		//0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
		this->setConversionRate(2);
		
		//set Extended Mode.
		//0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
		this->setExtendedMode(0);
	}

	return hasSensor;
}
