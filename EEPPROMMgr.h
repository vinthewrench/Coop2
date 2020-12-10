//
//  saved_events.h
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

#ifndef saved_events_h
#define saved_events_h

#include "ScheduleMgr.h"
#include "TimeLib.h"

typedef  struct{
	bool useDST;
	bool wifiOnStartup;
	bool wifiAuto;		// turn on wifi only when you need it
	bool remoteTermOnStartup;
	bool remoteRESTOnStartup;
}  pref_bits_entry;

class EEPPROMMgr {
	
public:
	
	EEPPROMMgr() {};
	
	bool isSetup();
	void clear_all();

	void set_lastTimeSync(time_t epoch); // in GMT time
	time_t get_lastTimeSync(); // in GMT time

	void set_timeSyncInterval(unsigned long seconds);	// how often do we do timesync
	unsigned long get_timeSyncInterval();

	void set_timeServerName(const char* dnsName);	// max 256 chars
	bool get_timeServerName(char** dnsNameAlloc );	// free when done
	
	bool store_event(uint8_t eventNum, SavedEvent event, tod_offset_t timeBase,  int16_t timeOfDay);
	bool get_event(uint8_t eventNum,  SavedEvent *event, tod_offset_t *timeBase, int16_t *timeOfDay);
	
	bool get_ssid(char ssid[32]);
	bool store_ssid(const char* ssid);
	
	bool get_wifiPwd(char password[64]);
	bool store_wifiPwd(const char* password);
	
	void set_prefBits(pref_bits_entry prefs);
	bool get_prefBits(pref_bits_entry *prefs);
	
	bool set_timezone(int tzOffset, double Latitude, double Longitude);    // Timezone  offset in Hours
	bool get_timezone(int *tzOffset, double *Latitude, double *Longitude);    // Timezone  offset in Hours
 
	void set_TempHighTrigger(float temperature, SavedEvent event);
	void set_TempLowTrigger(float temperature, SavedEvent event);
	bool get_TempHighTrigger(float *temperature, SavedEvent *event);
	bool get_TempLowTrigger(float *temperature, SavedEvent *event);

	bool get_remoteTermPort(uint16_t *portNum);
	bool set_remoteTermPort(uint16_t portNum);

	void set_backlight(unsigned long rgb);
	unsigned long get_backlight();
	
	

private:
	void  write_sig();
};


#endif /* saved_events_h */
