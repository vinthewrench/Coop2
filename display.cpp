//
//  display.c
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


#include "display.h"
#include "CommonIncludes.h"

const char* STR_NoInternet = "No Internet";

const char* STR_UNKNOWN = "Unknown";
const char* STR_OPEN = "Open";
const char* STR_CLOSED = "Closed";
const char* STR_OPENING = "Opening";
const char* STR_CLOSING = "Closing";

const char* STR_ON = "ON";
const char* STR_OFF= "OFF";

Display::Display() {
	hasDisplay = false;
}

bool Display::begin(TwoWire &wirePort, uint8_t i2c_addr){
	
	hasDisplay = false;
	_displayDimmed = false;

	_lcd = new SerLCD();
	
	// Setup LCD
	_lcd->begin(wirePort,i2c_addr);
	this->init();
	
	delay(1000);
	_lcd->setCursor(0, 0);
	_lcd->write( (uint8_t*)"Initializing...", 15);
	delay(1000);
	hasDisplay = true;

	return true;
}

void Display::init(){
	
	_lcd->display();
	delay(100);
	
	unsigned long rgb =  (unsigned long) BLColor::grey;
	if(eeMgr.isSetup()) {
		rgb = eeMgr.get_backlight();
		_lcd->setFastBacklight(rgb); //grey
	}
//	delay(500);

	_lcd->disableSystemMessages();
	_lcd->disableSplash();
	_lcd->noAutoscroll();
	_lcd->clear();
	delay(100);

  	//	_lcd->setContrast(5); //Set contrast. Lower to 0 for higher contrast.
	
}


BLColor Display::stringToColor(const char* str){
	BLColor color = BLColor::invalid;
	
	if(CMPSTR(str, "OFF")) 			color = BLColor::off;
	else if(CMPSTR(str, "ORANGE")) color = BLColor::orange;
	else if(CMPSTR(str, "GREY")) 	color = BLColor::grey;
	else if(CMPSTR(str, "WHITE")) 	color = BLColor::white;
	else if(CMPSTR(str, "RED")) 	color = BLColor::red;
	else if(CMPSTR(str, "BLUE")) 	color = BLColor::blue;
	else if(CMPSTR(str, "GREEN")) 	color = BLColor::green;
	else if(CMPSTR(str, "YELLOW")) 	color = BLColor::yellow;
	else if(CMPSTR(str, "INIDGO")) 	color = BLColor::indigo;
	else if(CMPSTR(str, "VIOLET")) 	color = BLColor::violet;
	else if(CMPSTR(str, "DIM")) 	color = BLColor::dim;
	
	return color;
}
 
void Display::setBackLightRGB(unsigned long rgb){
	if(!hasDisplay) return;
	_lcd->setFastBacklight(rgb);
}

void  Display::setBackLight(BLColor color){

	unsigned long rgb = (unsigned long) color;
	this->setBackLightRGB(rgb);
}

void Display::clear(){
	if(!hasDisplay) return;
	_lcd->clear();
	
}

void Display::println(const char* str){
	if(!hasDisplay || _displayDimmed) return;
	_lcd->println(str);
}


void Display::displayLinkStatus()
{
	//	EthernetLinkStatus linkStatus = Ethernet.linkStatus();
	
	if(!hasDisplay) return;
 
	char buffer [20];
	memset(buffer,' ' , sizeof(buffer));
	uint8_t len = 0;
	
	char timeState = ' ';
	int remoteCount = 0;

	const char *strIP = "";
	
	bool isConnected = wifiMgr.isConnectedToAP();
	if(isConnected){
		strIP = wifiMgr.ipAddr();
 		len = snprintf(buffer,sizeof(buffer), "%s", strIP);

	} else {
		
		int attempCount = 0;
		if(wifiMgr.isTryingToConnect(&attempCount)){
			if(attempCount == 0){
				len = snprintf(buffer,sizeof(buffer), "Connecting..");
			} else {
				len = snprintf(buffer,sizeof(buffer), "Retry:%d",attempCount);
			}
		}
		else {
			len = snprintf(buffer,sizeof(buffer), "Wifi:OFF");
		}
	}
		
	remoteCount = telnetPortMgr.activePortCount();
	
	buffer[len] = ' ';
	
	time_status status = tsMgr.timeSyncStatus();
	
	switch(status) {
		case TIME_NOT_SET:
			timeState = '?';
			break;
		case TIME_NEEDS_SYNC:
			timeState = 's';
			break;
		case TIME_SYNCED:
			timeState = 'S';
			break;
	}
	
	buffer[18] = remoteCount?'R': (telnetPortMgr._running?'r':' ');
	
	buffer[19] = timeState;
	
	_lcd->setCursor(0, 3);
	_lcd->write((const uint8_t*) buffer, sizeof(buffer));
}


void Display::displayPowerMgmt(){
	if(!hasDisplay) return;
 
	char buffer [20];
	memset(buffer,' ' , sizeof(buffer));
	uint8_t len = 0;

	if( powerMgr.hasSensor()){
		float  current =  powerMgr.average_current_mA();
		len = snprintf(buffer, sizeof(buffer), "%4.1fmA", current );
	}
	
	buffer[len] = ' ';
 
	_lcd->setCursor(0, 2);
	_lcd->write((const uint8_t*) buffer, sizeof(buffer));
}

void Display::displayTemp(){

	if(!hasDisplay) return;
 
	char buffer [20];
	memset(buffer,' ' , sizeof(buffer));
	uint8_t len = 0;
	
	if(tempSensor.hasSensor){
		float temperature = tempSensor.readTempF();
 
		len = snprintf(buffer, sizeof(buffer), "%3i\xDF%c", int(temperature), 'F');
		_lcd->setCursor(15, 1);
		_lcd->write((const uint8_t*)buffer, 5);
	}
}

 
void Display::displayClock(){
	
	if(!hasDisplay) return;
 
	char buffer [21];
	memset(buffer,' ' , sizeof(buffer));
	uint8_t len = 0;

	if(!rtc.isTimeSet()){
		
		_lcd->setCursor(0, 0);
		len = snprintf(buffer, sizeof(buffer), "- TIME IS NOT SET -");
		buffer[len] = ' ';

 		_lcd->write((const uint8_t*)buffer, sizeof(buffer) -1);
		return;
	}
	
	time_t  date =  tsMgr.timeNow();

	 	// digital clock display of the time
	
	len = snprintf(buffer, sizeof(buffer),
						"%d:%02d:%02d%s",
						hourFormat12(date), minute(date), second(date), isAM(date)?"am":"pm" );
	buffer[len] = ' ';
	
	//
	//	// SUN 06-24
	//
	uint8_t offset = 11;
	
	len = snprintf(buffer + offset, sizeof(buffer) - offset,
						"%3s %02u-%02u",
						dowStr(date),
						month(date),day(date) );
	buffer[len + offset] = ' ';
	
	_lcd->setCursor(0, 0);
	_lcd->write((const uint8_t*)buffer, sizeof(buffer) -1);
}



void Display::displayState()
{
	if(!hasDisplay) return;
 
	static state_t last_state = STATE_UNKNOWN;
	
	char buffer [20];
	memset(buffer,' ' , sizeof(buffer));
	uint8_t len = 0;
	
	state_t state = stateMgr.currentState;
	
	const char *str;
	
	switch(state) {
		case STATE_UNKNOWN:
			str = STR_UNKNOWN;
			break;
			
		case STATE_OPEN:
			str = STR_OPEN;
			break;
			
		case STATE_OPENING:
			str = STR_OPENING;
			break;
			
		case STATE_CLOSED:
			str = STR_CLOSED;
			break;
			
		case STATE_CLOSING:
			str = STR_CLOSING;
			break;
	}
	
	len = snprintf(buffer, sizeof(buffer), "%-8s L:%-3s",str,   door.light1()?STR_ON:STR_OFF);
	buffer[len] = ' ';
	
	_lcd->setCursor(0, 1);
	_lcd->write((const uint8_t*)buffer, 15);
	
	last_state = state;
}


void Display::updateDisplay(bool forceUpdate) {
	
	if(!hasDisplay) return;
 
	if(door.enclosureDoorClosed() && !_displayDimmed){
		_lcd->clear();
 		_lcd->noDisplay();
		_lcd->setFastBacklight(0);
		_displayDimmed = true;
		return;
	}
	
	if(!door.enclosureDoorClosed() && _displayDimmed){
		_displayDimmed = false;
		this->init();
		forceUpdate = true;
	}
 
	
	if(_displayDimmed)
		return;
	
	uint32_t now = millis();
	
	if(forceUpdate
		|| ( now > (_lastUpdate + 500))){
		displayState();
	}
	
	if(forceUpdate
		|| (now > (_lastUpdate + 1000))){
		displayClock();
		displayTemp();
		displayPowerMgmt();
		displayLinkStatus();
		
		_lastUpdate = millis();
	}
}
