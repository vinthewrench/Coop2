//
//  door.c
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

#include "door.h"
#include "CommonIncludes.h"
 
#define DOOR_RLY_1  1
#define DOOR_RLY_2  2
#define DOOR_RLY_3  3
#define DOOR_RLY_4  4

bool _doorstate = false;

bool Door::begin(uint8_t address,
					  uint8_t enclosureClosedPin,
					  uint8_t coopDoorClosedPin) {
	
	pinMode(enclosureClosedPin, INPUT);
	pinMode(enclosureClosedPin, INPUT_PULLUP);
	
	pinMode(coopDoorClosedPin, INPUT);
	pinMode(coopDoorClosedPin, INPUT_PULLUP);
	
	_enclosureClosedPin = enclosureClosedPin;
	_coopDoorClosedPin = coopDoorClosedPin;

	_relay = new Qwiic_Relay(address);
	
	hasDoor = _relay->begin();
	if(!hasDoor)
		return false;
		
	stop();
	
	return true;
}


bool Door::self_test() {
	
	if(!hasDoor) return false;
	
	for(int i = 0; i < 2;  i++){
		_relay->toggleRelay(1);
		delay(200);
		_relay->toggleRelay(2);
		delay(200);
		_relay->toggleRelay(3);
		delay(200);
		_relay->toggleRelay(4);
		delay(200);
	}
	_relay->turnAllRelaysOff();
	
	return true;
	
}

bool Door::enclosureDoorClosed() {
	return (digitalRead(_enclosureClosedPin) == LOW);
}

//bool Door::safetySwitch() {
//
//	return (digitalRead(doorSafetyPin) == LOW);
//}

bool Door::coopDoorisClosed() {
	return (digitalRead(_coopDoorClosedPin) == LOW);
}


void Door::open()
{
	if(!hasDoor) return;
	_relay->turnRelayOff(DOOR_RLY_2);
	_relay->turnRelayOn(DOOR_RLY_1);
	
	buttons.LEDblink(BUTTON_GREEN);
}

void Door::close()
{
	if(!hasDoor) return;
	
	_relay->turnRelayOff(DOOR_RLY_1);
	_relay->turnRelayOn(DOOR_RLY_2);
	buttons.LEDblink(BUTTON_RED);
}

void Door::stop()
{
	if(!hasDoor) return;
	_relay->turnRelayOff(DOOR_RLY_1);
	_relay->turnRelayOff(DOOR_RLY_2);
	
	buttons.LEDoff(BUTTON_RED);
	buttons.LEDoff(BUTTON_GREEN);
}


bool Door::light1(){
	
	if(!hasDoor) return false;
	
	uint8_t state =  _relay->getState(DOOR_RLY_3);
	return (state==1);
}

void Door::setLight1(bool isOn){
	
	if(!hasDoor) return;
	
	if (isOn) {
		_relay->turnRelayOn(DOOR_RLY_3);
	} else {
		_relay->turnRelayOff(DOOR_RLY_3);
	}
	
}

bool Door::relay4(){
	
	if(!hasDoor) return false;
	
	uint8_t state =  _relay->getState(DOOR_RLY_4);
	return (state==1);
}

void Door::setRelay4(bool isOn){
	
	if(!hasDoor) return;
	if (isOn) {
		_relay->turnRelayOn(DOOR_RLY_4);
	} else {
		_relay->turnRelayOff(DOOR_RLY_4);
	}
}

