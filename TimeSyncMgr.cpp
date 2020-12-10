//
//  timeSync.c
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

#include "CommonIncludes.h"
#include "TimeSyncMgr.h"
#include "TimeLib.h"

#include "RTC.h"



time_t lastResync = 0;

//constexpr time_t MAX_SYNC_INTERVAL_SECS =  60 * 10 ; //(1 * SECS_PER_DAY);
constexpr int MAX_NTP_TRIES  =	5;

 
bool TimeSyncMgr::init(){
	
	_bootMillis = millis();
	_bootTime 	= 0;
	
	_maxTimeSyncInterval = eeMgr.get_timeSyncInterval();
	
	if(_timeServerDNS){
		free(_timeServerDNS);
		_timeServerDNS = NULL;
	}
	eeMgr.get_timeServerName(&_timeServerDNS);
  	eeMgr.get_timezone(&_tzOffset, &_latitude, &_longitude);
 
	_cachedJD = 0;
	_ntpAttempts = 0;
	_ntpSyncInProgress = false;
	
	updatePrefs();
	
	//	DPRINTF("sun.setPosition(%f , %f, %f)" ,_latitude, _longitude, _tzOffset);
	
	/* Set our position and a default timezone value */
	sun.setPosition(_latitude, _longitude, _tzOffset);
	return true;
}


void TimeSyncMgr::updatePrefs(){
	pref_bits_entry prefs;
	eeMgr.get_prefBits(&prefs);
	_cachedJD = 0;
	_useDST = prefs.useDST;
	
	_maxTimeSyncInterval = eeMgr.get_timeSyncInterval();

}

 
bool TimeSyncMgr::isDST(time_t t) {
	
	if(_useDST){
		return  isUSDST(t);
	}
	return false;
}

/* time sync functions	*/



time_status TimeSyncMgr::timeSyncStatus(){
	
	unsigned long now = millis();
	
	if(!rtc.isTimeSet())
		return TIME_NOT_SET;
	
	if(lastResync == 0){
		
		time_t gmEEPrm  = eeMgr.get_lastTimeSync();
		
		if(gmEEPrm == 0)
			return TIME_NOT_SET;
		
		rtc.getTime();
		time_t gmRTC =  rtc.getEpoch();
		
		if( (gmRTC - gmEEPrm) > _maxTimeSyncInterval)
			return TIME_NEEDS_SYNC;
		
		lastResync = now;
		
		goto timeSynced;
	}
	
	if(now < (lastResync + (_maxTimeSyncInterval * 1000))){
		goto timeSynced;
	}
 
	return TIME_NEEDS_SYNC;
	
	timeSynced:
 	// calculate boot time
	// take the tine and subtract the difference between _bootMillis and now.
		if(_bootTime == 0){
			
			rtc.getTime();
			time_t gmRTC =  rtc.getEpoch();
		
			_bootTime =  gmRTC - ((now - _bootMillis) / 1000);
		}

	return TIME_SYNCED;
}


time_t TimeSyncMgr::timeNow()
{
	if(timeSyncStatus() == TIME_NOT_SET)
		return 0;
	
	rtc.getTime();
	return rtc.getEpoch();
}


unsigned long TimeSyncMgr::upTime(){
	if(!_bootTime)
	 return 0;

	time_t now = timeNow();
	if(now == 0)
		return 0;

	return (now - _bootTime);
}


//
//time_t TimeSyncMgr::gmtTime()
//{
//	if(timeSyncStatus() == TIME_NOT_SET)
//		return 0;
//
//	rtc.getTime();
//	return rtc.getEpoch() - ((double)_tzOffset * SECS_PER_HOUR);
//}

time_t TimeSyncMgr::sunSet()
{
	if(timeSyncStatus() == TIME_NOT_SET)
		return 0;
	
	time_t now = timeNow();
	if(now == 0)
		return 0;
	
	time_t midnight =  previousMidnight(now);
	
	// recacluate the date
	
	double jd = sun.cachedJulianDate();
	
	if(_cachedJD != jd || _cachedJD == 0){
		
		_cachedJD = jd;
		
// 		DPRINTF("setCurrentDate(%u,%u,%u)",year(now),  month(now), day(now) );
		
		sun.setCurrentDate(year(now),  month(now), day(now));
		sun.setTZOffset((double)_tzOffset + ( isDST(now)?1:0));
//   	DPRINTF("setTZOffset(%f)" ,(double)_tzOffset + ( isDST(now)?1:0));
		
	}
	
	return  midnight + (sun.calcSunset() * 60);
}



time_t TimeSyncMgr::sunRise()
{
	if(timeSyncStatus() == TIME_NOT_SET)
		return 0;
	
	time_t now = timeNow();
	if(now == 0)
		return 0;
	
	time_t midnight =  previousMidnight(now);
	
	// recacluate the date
	if(_cachedJD != sun.cachedJulianDate() || _cachedJD == 0){

		sun.setCurrentDate(year(now),  month(now), day(now));
		sun.setTZOffset((double)_tzOffset + ( isDST(now)?1:0));
	}
	
	return  midnight + (sun.calcSunrise() * 60);
	
}


time_t TimeSyncMgr::civilSunSet()
{
	if(timeSyncStatus() == TIME_NOT_SET)
		return 0;
	
	time_t now = timeNow();
	if(now == 0)
		return 0;
	
	time_t midnight =  previousMidnight(now);
	
	// recacluate the date
	if(_cachedJD != sun.cachedJulianDate() || _cachedJD == 0){

		sun.setCurrentDate(year(now),  month(now), day(now));
		sun.setTZOffset((double)_tzOffset + ( isDST(now)?1:0));
	}
	
	return  midnight + (sun.calcCivilSunset() * 60);
	
}


time_t TimeSyncMgr::civilSunRise()
{
	if(timeSyncStatus() == TIME_NOT_SET)
		return 0;
	
	time_t now = timeNow();
	if(now == 0)
		return 0;
	
	time_t midnight =  previousMidnight(now);
	
	// recacluate the date
	if(_cachedJD != sun.cachedJulianDate() || _cachedJD == 0){

		sun.setCurrentDate(year(now),  month(now), day(now));
		sun.setTZOffset((double)_tzOffset + ( isDST(now)?1:0));
	}
	
	return  midnight + (sun.calcCivilSunrise() * 60);
	
}


void TimeSyncMgr::syncClockIfNeeded(bool forceSync)
{
	if 	(_ntpSyncInProgress)
		return;
	
	time_status status = timeSyncStatus();
	
	if( forceSync || status != TIME_SYNCED) {
		
		// if the network is not there. turn it on
		// and it will call  syncClockIfNeeded once it's connected
		
		if(!wifiMgr.hasInterNet){
			
			pref_bits_entry prefs;
			eeMgr.get_prefBits(&prefs);
			if(prefs.wifiAuto || prefs.wifiOnStartup){
				_shutWifiAfterSync = true;
				wifiMgr.setAPConnectionONwithOption(WFOPT_NTP_ONLY);
			}
		} else {
			
			_ntpAttempts = 0;
			_ntpSyncInProgress = true;
			
			if( this->startNTPSync()) {
				return;
			}
			
		}
	}
	
	_ntpSyncInProgress = false;
}

// MARK:  - NTP

void ntpCB(uint8_t link_id,
					  void*		arg,
					  int  		status,
					  char			ipaddr[16],
					  int 			portNum,
			const  uint8_t* 	data,
					  size_t		dataLen){
	
	TimeSyncMgr* ts = (TimeSyncMgr*) arg;
	
	if(ts){
		ts->ntpCallback(link_id, status, ipaddr, portNum, data, dataLen);
	}
}

void TimeSyncMgr::ntpFail() {
	
	cmdLineMgr.print( "NTP FAILED after %d attempts", _ntpAttempts);
	_ntpAttempts = 0;
	_ntpSyncInProgress = false;
	
	this->cleanupNTPSync();

	// too many retries?
	// faile
}

void TimeSyncMgr::ntpCallback(uint8_t link_id,
							int  		status,
							char		ipaddr[16],
							int 		portNum,
					const	uint8_t*	data,
							size_t	dataLen){
	
	if(status == WFCF_TIMEOUT){
	
		
		cmdLineMgr.print( "TIMEOUT [%d] %s %d",
						  link_id,
						  ipaddr, portNum);
		
		wifiMgr.closeConnection(link_id);
		
		// fire the connection again
		if(_ntpAttempts <  MAX_NTP_TRIES){
			this->startNTPSync();
		}
		else {
			this->ntpFail();
		}
		
	} else if(status == WFCF_CLOSED){
			
		Serial_Printf(&Serial, "CLOSED [%d] ",link_id );
		
	}
	else if(status == WFCF_IPD){
//
//				Serial_Printf(&Serial, "RECV [%d] len:%ld, %s %d",
//								  link_id,
//								  dataLen,
//								  ipaddr, portNum);
//
		//		dumpHex(&Serial,(unsigned char*) data, (int) dataLen, 0);
		
		
		// was the packet OK?
		if(dataLen != 48) {
			
			wifiMgr.closeConnection(link_id);
			
			if(_ntpAttempts <  MAX_NTP_TRIES){
				this->startNTPSync();
			}
			else {
				this->ntpFail();
			}
			return;
		}
		
		constexpr unsigned long SEVENZYYEARS  = 2208988800UL;
		
		unsigned long highWord = word(data[40],  data[41]);
		unsigned long lowWord = word(data[42],  data[43]);
		// combine the four bytes (two words) into a long integer
		// this is NTP time (seconds since Jan 1 1900):
		unsigned long secsSince1900 = highWord << 16 | lowWord;
		
		unsigned long ntpTime = secsSince1900 - SEVENZYYEARS;
		
		
		time_t localTime = ntpTime  + (_tzOffset * SECS_PER_HOUR);
		
		if(isDST(localTime))
			localTime += SECS_PER_HOUR;
		
		rtc.setEpoch(localTime);
		
		eeMgr.set_lastTimeSync(localTime);
		
		lastResync = millis();
		
	
		cmdLineMgr.print("SYNC TIME %2d-%02d-%4d %02d:%02d:%02d %2s %3s" ,
							  month(localTime), day(localTime), year(localTime), hourFormat12(localTime),
							  minute(localTime), second(localTime),
							  isAM(localTime)?"AM":"PM", isDST(localTime)?"DST":"ST");
		
		_ntpAttempts = 0;
		_ntpSyncInProgress = false;
		
		wifiMgr.closeConnection(link_id);
		
		this->cleanupNTPSync();
	}
	else
	{
		Serial_Printf(&Serial, "NTP CALLBACK [%d] ",link_id );
		
	}
}


void TimeSyncMgr::cleanupNTPSync(){
	if(_shutWifiAfterSync) {
		 wifiMgr.setAPConnection(false);
		_shutWifiAfterSync = false;
	}
}

bool TimeSyncMgr::startNTPSync(){
	
	uint8_t  linkID;
		
	if(! wifiMgr.openConnection(false, _timeServerDNS ,123,ntpCB,this, &linkID))
		return false;
 
	const uint8_t NTP_PACKET_SIZE= 48; // NTP time stamp is in the first 48 bytes of the message
	
	uint8_t packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
	// set all bytes in the buffer to 0
	bzero(packetBuffer, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12]  = 49;
	packetBuffer[13]  = 0x4E;
	packetBuffer[14]  = 49;
	packetBuffer[15]  = 52;
	
	if(!wifiMgr.sendUDP(linkID, 1000, packetBuffer, NTP_PACKET_SIZE ))
		return false;

	_ntpAttempts++;
	
	// delay just a sec in case we respond right away
	for(int i = 0; i < 100; i++){
		wifiMgr.loop();
		
		if(!wifiMgr.isConnectedToAP()){
			return false;
		}
		
		delay(5);
	}

	return true;
}
