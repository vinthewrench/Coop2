//
//  door.h
//  clocktest
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

#ifndef door_h
#define door_h

#include <Arduino.h>

#include "SparkFun_Qwiic_Relay.h"

class Door
{  
public:
	
	bool begin(uint8_t address,
				  uint8_t enclosureClosedPin,
				  uint8_t coopDoorClosedPin );

	bool self_test();
	
	void open();
	void close();
	void stop();
	
	void setLight1(bool isOn);
   bool light1();

	void setRelay4(bool isOn);
   bool relay4();

//	bool  safetySwitch();
	bool  coopDoorisClosed();
	bool  enclosureDoorClosed();
	
	bool hasDoor;
	
private:
	
 	Qwiic_Relay *_relay;
	uint8_t 		_enclosureClosedPin;
	uint8_t 		_coopDoorClosedPin;

};

#endif /* door_h */
