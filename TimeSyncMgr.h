//
//  timeSync.h
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

#ifndef timeSync_h
#define timeSync_h

#include "sunset.h"

enum time_status {
	TIME_NOT_SET = 0,
	TIME_NEEDS_SYNC,
	TIME_SYNCED
};
 
class TimeSyncMgr {
	
public:
	
	TimeSyncMgr() {};
	
	bool init();	// read prefs from eeprom
	
//	bool init(int tzOffset, double Latitude, double Longitude);    // Timezone  offset in Hours
	
	void updatePrefs();
	
	/* time sync functions	*/
	time_status timeSyncStatus(); // indicates if time has been set and recently synchronized
	
 	void syncClockIfNeeded(bool forceSync = false);

	void  ntpCallback(uint8_t link_id, int status, char ipaddr[16], int portNum,
							const uint8_t* data, size_t dataLen);
	
	
	void  cleanupNTPSync();

	bool  isDST(time_t t);		// uses prefs to see if we observe it

	time_t timeNow();
	time_t sunSet();
	time_t sunRise();
	time_t civilSunSet();
	time_t civilSunRise();
	unsigned long  upTime();	// elapsed time since last boot
 
	double 	_latitude;
	double 	_longitude;
	int			_tzOffset;

private:

	SunSet 	sun;
	double		_cachedJD;
	bool 		_useDST;
	
unsigned long	_bootMillis;		// used to track when the system booted.
	time_t		_bootTime;			// calculate after we do our first timesync.
	
unsigned long _maxTimeSyncInterval;
	char*		_timeServerDNS;		//  must free when done;
	int			_ntpAttempts;
	bool 		_ntpSyncInProgress;
	
	bool		_shutWifiAfterSync;

	bool  		startNTPSync();
	void  		ntpFail();
	
	

	
};


 
#endif /* timeSync_h */
