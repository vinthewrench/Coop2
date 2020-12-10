//
//  stored_events.c
//  chickencoopArt
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


#include <EEPROM.h>

#include "time.h"
#include "EEPPROMMgr.h"
#include "CommonIncludes.h"


typedef struct  {
	int tzOffset;
	double Latitude;
	double Longitude;
} 	tz_entry;
 
#define EPROM_OFFSET_SIG  			0

#define EPROM_OFFSET_LAST_TIMESYNC  8

#define EPROM_OFFSET_EVENTS  (EPROM_OFFSET_LAST_TIMESYNC + sizeof(time_t))

#define EPROM_OFFSET_SSID		(EPROM_OFFSET_EVENTS + (MAX_STORED_EVENTS * 4))

#define EPROM_OFFSET_WIFIPWD	(EPROM_OFFSET_SSID + 32)

#define EPROM_OFFSET_TZ 		(EPROM_OFFSET_WIFIPWD + 64)
 
#define EPROM_OFFSET_PREFS	(EPROM_OFFSET_TZ + sizeof(tz_entry))
 
#define EPROM_OFFSET_TEMP_LOW		(EPROM_OFFSET_PREFS + sizeof(uint32_t))

#define EPROM_OFFSET_TEMP_HIGH	(EPROM_OFFSET_TEMP_LOW + sizeof(temperature_entry))

#define EPROM_OFFSET_REMOTE_PORT			(EPROM_OFFSET_TEMP_HIGH + sizeof(temperature_entry) )
 
#define EPROM_OFFSET_BACKLIGHT	  		(EPROM_OFFSET_REMOTE_PORT + sizeof(unsigned long) )

#define EPROM_OFFSET_TIMESYNCINTERVAL		(EPROM_OFFSET_BACKLIGHT + sizeof(unsigned long) )

#define EPROM_OFFSET_TIMESERVER_DNS		(EPROM_OFFSET_TIMESYNCINTERVAL + sizeof(unsigned long ))

#define EPROM_OFFSET_NEXT			(EPROM_OFFSET_TIMESERVERDNS + 256 )		/* stored as counted string */
 

#define EPROM_SIG	0x4D41474132303230

void EEPPROMMgr::write_sig() {
	uint64_t sig = EPROM_SIG;
	EEPROM.put( EPROM_OFFSET_SIG, sig);
}

bool EEPPROMMgr::isSetup() {
	uint64_t sig = 0;
	EEPROM.get( EPROM_OFFSET_SIG, sig);
	
	return (sig == EPROM_SIG);
}

 
void EEPPROMMgr::set_backlight(unsigned long rgb){
	EEPROM.put( EPROM_OFFSET_BACKLIGHT, rgb);
}

unsigned long EEPPROMMgr::get_backlight()
{
	unsigned long rgb;
	EEPROM.get( EPROM_OFFSET_BACKLIGHT, rgb);
	return  rgb;
}


void EEPPROMMgr::set_lastTimeSync(time_t epoch) {
	
	EEPROM.put( EPROM_OFFSET_LAST_TIMESYNC, epoch);
}

time_t EEPPROMMgr::get_lastTimeSync() {
	
	time_t data = 0;
	EEPROM.get(EPROM_OFFSET_LAST_TIMESYNC, data);
	
	return data;
}

// how often do we do timesync
void EEPPROMMgr::set_timeSyncInterval(unsigned long seconds){

	EEPROM.put( EPROM_OFFSET_TIMESYNCINTERVAL, seconds);
}

unsigned long EEPPROMMgr::get_timeSyncInterval(){
	
	unsigned long syncInterval = 0;
	EEPROM.get( EPROM_OFFSET_TIMESYNCINTERVAL, syncInterval);
	return syncInterval;
}

void EEPPROMMgr::set_timeServerName(const char* dnsName){ // max 255 chars
	char buffer[256];
	
	strncpy(buffer, dnsName, sizeof(buffer)-1);
	buffer[255] = 0;
	
	EEPROM.put( EPROM_OFFSET_TIMESERVER_DNS , buffer);
}

bool EEPPROMMgr::get_timeServerName(char** dnsNameAlloc ){ 	// free when done
	char buffer[256];
	
	EEPROM.get( EPROM_OFFSET_TIMESERVER_DNS, buffer);

	if(dnsNameAlloc){
		*dnsNameAlloc = strndup(buffer, sizeof(buffer));
	}
	return true;
}

 
void EEPPROMMgr::clear_all()
{
	// clear events
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++)
		store_event(i, SE_NONE,TOD_INVALID, 0);
	 
	store_ssid("");
	store_wifiPwd("");
	set_remoteTermPort(0);
	
	pref_bits_entry prefs;
	prefs.useDST = true;
	prefs.remoteTermOnStartup = false;
	prefs.wifiOnStartup = false;
	prefs.wifiAuto = false;
 	set_prefBits(prefs);
	
	set_timezone(0,65.405,-161.2783);
	
	set_TempHighTrigger(0,SE_NONE);
	set_TempLowTrigger(0,SE_NONE);
	
	set_backlight(0x808080);
	set_timeSyncInterval(1 * SECS_PER_DAY);
	set_timeServerName("north-america.pool.ntp.org");
	this->write_sig();
}


bool EEPPROMMgr::store_event(uint8_t eventNum, SavedEvent event,
									  tod_offset_t timeBase, int16_t timeOfDay) {
	
	if( eventNum > MAX_STORED_EVENTS )
		return false;
	
	uint16_t tod = timeOfDay;
	
	uint32_t data =  ((uint8_t)event & 0xF) << 14
	| ((timeBase & 0x7) << 11)
	|  (tod & 0x7FF);
	
	//	{
	//   char buff[80];
	//
	//		sprintf(buff, "STORE EVT[%u] = %04X (%5s, %u, (%03X, %03X) )",  eventNum, data,
	//				  evenStr[event], timeBase, tod, (tod & 0x7FF));
	//
	//        Serial.println(buff);
	//	}
	
	EEPROM.put( EPROM_OFFSET_EVENTS + (eventNum * 4), data);
	return true;
	
}

bool EEPPROMMgr::get_event(uint8_t eventNum,  SavedEvent *eventOUT,
									tod_offset_t *timeBaseOut, int16_t *timeOfDayOUT){
	
	uint32_t data =  0;
	SavedEvent event = SE_NONE;
	int16_t tod = 0;
	
	tod_offset_t timeBase = TOD_INVALID;
	
	EEPROM.get(EPROM_OFFSET_EVENTS + (eventNum * 4), data); //(location, data));
	
	event = (SavedEvent) ((data >> 14) & 0x0f);
	timeBase = (tod_offset_t) ((data >> 11) & 0x07);
	tod =  (data & 0x7FF);
	
	if(tod > 1440) {
		tod =  (int16_t) (tod | 0xf800);
	}
	
	//	{
	//   char buff[80];
	//
	//		sprintf(buff, "GET EVT[%u] = %04X (%5s, %u, %03X)",  eventNum, data,
	//				  evenStr[event], timeBase, tod);
	//
	//        Serial.println(buff);
	//	}
	
	
	if(eventOUT) *eventOUT = event;
	if(timeBaseOut) *timeBaseOut = timeBase;
	if(timeOfDayOUT) *timeOfDayOUT = tod;
	
	return event != SE_NONE;
}


bool EEPPROMMgr::store_ssid(const char* ssid)
{
 	char data[32] = {0};
	 
	strncpy(data, ssid, sizeof(data));
 	EEPROM.put( EPROM_OFFSET_SSID  , data);
 
	//	debug_printf( "SET SSID = |%s| ", password?password:"<none>");

	return true;
}

bool EEPPROMMgr::store_wifiPwd(const char* password)
{
	char data[64] = {0};
	
	if(password){
		strncpy(data, password, sizeof(data));
	}
	
	EEPROM.put( EPROM_OFFSET_WIFIPWD  , data);
	
//	debug_printf( "SET PASSWORD = |%s| ", password?password:"<none>");
	
	return true;
	
}


bool EEPPROMMgr::get_ssid(char ssid[32])
{
	char data[32] = {0};

	EEPROM.get(EPROM_OFFSET_SSID , data);
	strncpy(ssid, data, sizeof(data));
 
	return true;
}

bool EEPPROMMgr::get_wifiPwd(char password[64])
{
	char data[64] = {0};

	  EEPROM.get(EPROM_OFFSET_WIFIPWD , data);
	  strncpy(password, data, sizeof(data));
	
	return true;
}

  
bool EEPPROMMgr::set_timezone(int tzOffset, double Latitude, double Longitude)
{
	tz_entry tz;
	tz.tzOffset = tzOffset;
	tz.Latitude = Latitude;
	tz.Longitude = Longitude;
	
	EEPROM.put( EPROM_OFFSET_TZ  , tz);
 
	return true;
}

bool EEPPROMMgr::get_timezone(int *tzOffset, double *Latitude, double *Longitude)
{
	tz_entry tz;

	EEPROM.get( EPROM_OFFSET_TZ  , tz);

	if(tzOffset) *tzOffset = tz.tzOffset;
	if(Latitude) *Latitude = tz.Latitude;
	if(Longitude) *Longitude = tz.Longitude;
 
	return true;
}


void EEPPROMMgr::set_prefBits(pref_bits_entry prefs)
{
	uint32_t data = 0;
 
	if(prefs.useDST) 					data |= 0x1;
	if(prefs.wifiOnStartup) 			data |= 0x2;
	if(prefs.remoteTermOnStartup) data |= 0x4;
	if(prefs.wifiAuto) 			data |= 0x8;
	if(prefs.remoteRESTOnStartup) data |= 0x10;
	
	EEPROM.put( EPROM_OFFSET_PREFS  , data);
}

bool EEPPROMMgr::get_prefBits(pref_bits_entry *prefsOut)
{
	uint32_t data;
	pref_bits_entry prefs = {false,false,false, false};
 
	EEPROM.get( EPROM_OFFSET_PREFS  , data);

	if(data & 0x1) prefs.useDST = 1;
	if(data & 0x2) prefs.wifiOnStartup = 1;
	if(data & 0x4) prefs.remoteTermOnStartup = 1;
	if(data & 0x8) prefs.wifiAuto = 1;
	if(data & 0x10) prefs.remoteRESTOnStartup = 1;
 
	if(prefsOut) *prefsOut = prefs;
	return true;
}

void EEPPROMMgr::set_TempHighTrigger(float temperature, SavedEvent event)
{
	temperature_entry t = {temperature,event};
	EEPROM.put( EPROM_OFFSET_TEMP_HIGH  , t);

}

void EEPPROMMgr::set_TempLowTrigger(float temperature, SavedEvent event)
{
	temperature_entry t = {temperature,event};
	EEPROM.put( EPROM_OFFSET_TEMP_LOW  , t);

}

bool EEPPROMMgr::get_TempHighTrigger(float *temperature, SavedEvent *event)
{
	temperature_entry t;

	EEPROM.get( EPROM_OFFSET_TEMP_HIGH, t);
	
	if(!isValidEvent(t.event))
		t.event = SE_NONE;

	if(temperature) *temperature = t.temperature;
	if(event) *event = t.event;
	return true;

}

bool EEPPROMMgr::get_TempLowTrigger(float *temperature, SavedEvent *event)
{
	temperature_entry t;

	EEPROM.get( EPROM_OFFSET_TEMP_LOW  , t);
	
	if(!isValidEvent(t.event))
		t.event = SE_NONE;

	if(temperature) *temperature = t.temperature;
	if(event) *event = t.event;
	return true;
}

 

bool EEPPROMMgr::set_remoteTermPort(uint16_t portNum)
{
	EEPROM.put( EPROM_OFFSET_REMOTE_PORT, portNum);
	
 	return true;
}


bool EEPPROMMgr::get_remoteTermPort(uint16_t *portNum){
	uint16_t data;

	EEPROM.get( EPROM_OFFSET_REMOTE_PORT, data);
	
	if(portNum) *portNum = data;
 	return true;
}



