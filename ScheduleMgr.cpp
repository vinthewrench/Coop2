//
//  ScheduleMgr.cpp
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


#include "ScheduleMgr.h"
#include "CommonIncludes.h"
#include <search.h> // for the qsort()
#include <limits.h>



char const* evenStr[] =
{ "NONE", "RELOAD", "OPEN","CLOSE", "L1 ON", "L1 OFF", "RL4 ON" , "RL4 OFF", "EVT8", "EVT9", "EVT10"} ;

typedef struct  {
	uint8_t 		eventNum;
	SavedEvent 	event;
	tod_offset_t	timeBase;
	int16_t 		timeOfDay;
	time_t  		lastRunTime;
} 	scheduleEntry_t;

scheduleEntry_t scheduleTable[MAX_STORED_EVENTS];

typedef struct  {
	int16_t sunriseMins;
	int16_t sunsetMin;
	int16_t civilSunsetMin;
	int16_t civilSunRiseMin;
	
}sunTimes_t;

int16_t actualTimeOfDay(scheduleEntry_t* entry, sunTimes_t *sp){
	
	int16_t actualTime = entry->timeOfDay;
	
	switch(entry->timeBase){
		case TOD_SUNRISE:
			actualTime = sp->sunriseMins + actualTime;
			break;
			
		case TOD_SUNSET:
			actualTime = sp->sunsetMin + actualTime;
			break;
			
		case TOD_CIVIL_SUNRISE:
			actualTime = sp->civilSunRiseMin + actualTime;
			break;
			
		case TOD_CIVIL_SUNSET:
			actualTime = sp->civilSunsetMin + actualTime;
			break;
			
		case TOD_ABSOLUTE:
			break;
			
		default:
			break;
	}
	
	return actualTime;
}


int CompareByscheduledTimeOfDay(void* arg,  const void *elem1, const void *elem2 )
{
	
	int16_t scheduledTimeOfDay1 = actualTimeOfDay((scheduleEntry_t*)elem1,(sunTimes_t*)arg);
	int16_t scheduledTimeOfDay2 = actualTimeOfDay((scheduleEntry_t*)elem2,(sunTimes_t*)arg);
	
	return (scheduledTimeOfDay1 > scheduledTimeOfDay2 ? 1 : -1);
}


bool ScheduleMgr::loadSchedule() {
	
	uint8_t count = 0;
	
	// Serial.println("-loadSchedule-");
	
	time_t  now =  tsMgr.timeNow();
	if(now == 0){
		return false;
	}
	
	sunTimes_t sp;
	sp.sunriseMins 	=  elapsedSecsToday(tsMgr.sunRise()) / 60;
	sp.sunsetMin 		=  elapsedSecsToday(tsMgr.sunSet()) / 60;
	sp.civilSunRiseMin =  elapsedSecsToday(tsMgr.civilSunRise()) / 60;
	sp.civilSunsetMin  =  elapsedSecsToday(tsMgr.civilSunSet()) / 60;
	
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++) {
		scheduleTable[i] = {UCHAR_MAX,SE_NONE, TOD_INVALID,0,0};
	}
	
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++){
		
		SavedEvent event = SE_NONE;
		int16_t  todMin = 0;
		tod_offset_t timeBase = TOD_INVALID;
		
		time_t scheduled = 0;
		
		if( eeMgr.get_event(i,&event, &timeBase, &todMin ))
		{
			scheduleTable[count].eventNum = i;
			scheduleTable[count].event = event;
			scheduleTable[count].timeBase = timeBase;
			scheduleTable[count].timeOfDay = todMin;
			scheduleTable[count].lastRunTime = 0;
			count ++;
		}
	}
	
	qsort_r(scheduleTable, count, sizeof(scheduleEntry_t), &sp,
			  CompareByscheduledTimeOfDay);
	
	eeMgr.get_TempLowTrigger( &_lowTemp.temperature, &_lowTemp.event );
	eeMgr.get_TempHighTrigger( &_highTemp.temperature, &_highTemp.event );
	
	_didLowTemp = false;
	_didHighemp = false;
	
	tableLoaded = true;
	
	return true;
}


bool ScheduleMgr::peekNextEvent(SavedEvent *eventOut, uint32_t *scheduledTimeOut){
	
	time_t  now =  tsMgr.timeNow();
	if(now == 0){
		return false;
	}
	
	time_t midnight	=  previousMidnight(now);
	
	sunTimes_t sp;
	sp.sunriseMins  		=  elapsedSecsToday(tsMgr.sunRise()) / 60;
	sp.sunsetMin     		=  elapsedSecsToday(tsMgr.sunSet()) / 60;
	sp.civilSunRiseMin 	=  elapsedSecsToday(tsMgr.civilSunRise()) / 60;
	sp.civilSunsetMin	 	=  elapsedSecsToday(tsMgr.civilSunSet()) / 60;
	
	if(!tableLoaded) {
		loadSchedule();
	}
	
	// check for timed event
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++) {
		
		scheduleEntry_t entry = scheduleTable[i];
		
		// Is there an event
		if(entry.event == SE_NONE) continue;
		
		time_t when =  (((time_t) actualTimeOfDay(&entry, &sp)) *60 ) + midnight;
		
		// Is it in the future
		if(when >= now){
			
			if( elapsedDays(entry.lastRunTime) == elapsedDays(now)) continue;

			if(eventOut) *eventOut = entry.event;
			if(scheduledTimeOut) *scheduledTimeOut = when;

			return true;
		}
		
 	}
	
	return false;

}


bool ScheduleMgr::getNextEvent(SavedEvent *eventOut, uint32_t *scheduledTimeOut) {
	
	time_t  now =  tsMgr.timeNow();
	if(now == 0){
		return false;
	}
	
	time_t midnight	=  previousMidnight(now);
	
	sunTimes_t sp;
	sp.sunriseMins  		=  elapsedSecsToday(tsMgr.sunRise()) / 60;
	sp.sunsetMin     		=  elapsedSecsToday(tsMgr.sunSet()) / 60;
	sp.civilSunRiseMin 	=  elapsedSecsToday(tsMgr.civilSunRise()) / 60;
	sp.civilSunsetMin	 	=  elapsedSecsToday(tsMgr.civilSunSet()) / 60;
	
	if(!tableLoaded) {
		loadSchedule();
	}
	
	// check for timed event
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++) {
		
		scheduleEntry_t entry = scheduleTable[i];
		
		// Is there an event
		if(entry.event == SE_NONE) continue;
		
		time_t when =  (((time_t) actualTimeOfDay(&entry, &sp)) *60 ) + midnight;
		
		// Is it in the future
		if(when >= now)  continue;
		
		// did we run it today?
		if( elapsedDays(entry.lastRunTime) == elapsedDays(now)) continue;
		
		if(eventOut) *eventOut = entry.event;
		if(scheduledTimeOut) *scheduledTimeOut = when;
		
		scheduleTable[i].lastRunTime = now;
		return true;
	}
	
	// check for temp event
	if(tempSensor.hasSensor){
		bool didRead = false;
		float temperature;
		
		const float hysteresis = .5;
		
		/// WE MIGHT HAVE TO COOK IN SOME HYSTERIS HERE
		if(_lowTemp.event != SE_NONE)
		{
			if(!didRead)
				temperature = tempSensor.readTempF();
			
			if( temperature <= _lowTemp.temperature){
				
				if(!_didLowTemp){
					_didLowTemp = true;
					
					if(eventOut) *eventOut = _lowTemp.event;
					if(eventOut) *scheduledTimeOut = now;
					return true;
				}
				
			}
			else if( temperature > _lowTemp.temperature + hysteresis){
				if(_didLowTemp){
					_didLowTemp = false;
					
					SavedEvent iEVT = inverseEvent(_lowTemp.event);
					if(isValidEvent(iEVT))
					{
						if(eventOut) *eventOut = iEVT;
						if(scheduledTimeOut) *scheduledTimeOut = now;
						return true;
					}
				}
			}
		}
		
		if( _highTemp.event != SE_NONE)
		{
			if(!didRead)
				temperature = tempSensor.readTempF();
			
			if( temperature >= _highTemp.temperature){
				
				if(!_didHighemp){
					_didHighemp = true;
					
					if(eventOut) *eventOut = _highTemp.event;
					if(scheduledTimeOut) *scheduledTimeOut = now;
					return true;
				}
				
			}
			else if( temperature < _highTemp.temperature - hysteresis){
				
				if(_didHighemp){
					_didHighemp = false;
					
					SavedEvent iEVT = inverseEvent(_highTemp.event);
					if(isValidEvent(iEVT))
					{
						if(eventOut) *eventOut = iEVT;
						if(scheduledTimeOut) *scheduledTimeOut = now;
						return true;
					}
					
				}
			}
		}
	}
	
	return false;
}

bool ScheduleMgr::reconcileEvents(SavedEvent *events,  size_t inCount,  size_t *outCount ){
	
	size_t count = 0;
	
	SavedEvent lastDoorEvent = SE_NONE;
	SavedEvent lastLightEvent = SE_NONE;
	SavedEvent lastRelayEvent = SE_NONE;
	
	if(inCount < 2) return false;
	
	for(int i = 0; i < inCount; i++){
		events[i] = SE_NONE;
	}
	
	SavedEvent event = SE_NONE;
	uint32_t   when = 0;
	
	while(getNextEvent(&event, &when)){
		
		if(count > inCount) break;
		
		if(event == SE_NONE) continue;
		
		if(event == SE_DOOR_OPEN ||  event == SE_DOOR_CLOSE){
			lastDoorEvent = event;
		}
		else if(event == SE_LIGHT1_ON ||  event == SE_LIGHT1_OFF){
			lastLightEvent = event;
		}
		else if(event == SE_RELAY4_ON ||  event == SE_RELAY4_OFF){
			lastRelayEvent = event;
		}
		else {
			events[count++] = event;
		}
	}
	
	if( lastDoorEvent != SE_NONE){
		events[count++] = lastDoorEvent;
	}
	
	// default light to off
	if( lastLightEvent == SE_NONE){
		lastLightEvent = SE_LIGHT1_OFF;
	}
	events[count++] = lastLightEvent;
 
	// default relay to off
	if( lastRelayEvent == SE_NONE){
		lastRelayEvent = SE_RELAY4_OFF;
	}
	events[count++] = lastRelayEvent;

	if(outCount) *outCount = count;
	
	return true;
}


SavedEvent ScheduleMgr::inverseEvent(SavedEvent event){
	
	SavedEvent evt = SE_INVALID;
	
	switch(event){
		case SE_DOOR_OPEN: evt = SE_DOOR_CLOSE; break;
		case SE_DOOR_CLOSE: evt = SE_DOOR_OPEN; break;
		case SE_LIGHT1_ON: evt = SE_LIGHT1_OFF; break;
		case SE_LIGHT1_OFF: evt = SE_LIGHT1_ON; break;
		case SE_RELAY4_ON: evt = SE_RELAY4_OFF; break;
		case SE_RELAY4_OFF: evt = SE_RELAY4_ON; break;
		default: break;
	}
	
	return evt;
}

const char* entryTimeStr(scheduleEntry_t* entry){
	
	const char* str = "???";
	
	switch(entry->timeBase){
		case TOD_SUNRISE:
			str = "SR";
			break;
			
		case TOD_SUNSET:
			str = "SS";
			break;
			
		case TOD_CIVIL_SUNRISE:
			str = "CR";
			break;
			
		case TOD_CIVIL_SUNSET:
			str = "CS";
			break;
			
		case TOD_ABSOLUTE:
			str = "";
			break;
			
		default:
			break;
	}
	
	return str;
}


void ScheduleMgr::printSchedule(Stream *serial) {
	
	time_t  now =  tsMgr.timeNow();
	if(now == 0){
		return;
	}
	
	sunTimes_t sp;
	sp.sunriseMins 		=  elapsedSecsToday(tsMgr.sunRise()) / 60;
	sp.sunsetMin 	   	=  elapsedSecsToday(tsMgr.sunSet()) / 60;
	sp.civilSunRiseMin 	=  elapsedSecsToday(tsMgr.civilSunRise()) / 60;
	sp.civilSunsetMin	 	=  elapsedSecsToday(tsMgr.civilSunSet()) / 60;
	time_t midnight  		=  previousMidnight(now);
	
	if(!tableLoaded) {
		loadSchedule();
	}
	
	Serial_Printf(serial, "---Event Table---");
	for( uint8_t i = 0; i < MAX_STORED_EVENTS; i++) {
		char buff[80];
		char offsetBuf[40];
		
		scheduleEntry_t evt =  scheduleTable[i];
		
		if(evt.event  == SE_NONE) continue;
		
		time_t when =  (((time_t) actualTimeOfDay(&evt, &sp)) *60 ) + midnight;
		time_t lastRun = evt.lastRunTime;
		
		int length = 0;
		
		switch(evt.timeBase){
			case TOD_SUNRISE:
				sprintf(offsetBuf, "%s%-4d" , evt.timeOfDay>=0?"SR+":"SR", evt.timeOfDay);
				break;
				
			case TOD_SUNSET:
				sprintf(offsetBuf, "%s%-4d" , evt.timeOfDay>=0?"SS+":"SS", evt.timeOfDay);
				break;
				
			case TOD_CIVIL_SUNRISE:
				sprintf(offsetBuf,"%s%-4d" , evt.timeOfDay>=0?"CR+":"CR", evt.timeOfDay);
				break;
				
			case TOD_CIVIL_SUNSET:
				sprintf(offsetBuf, "%s%-4d" , evt.timeOfDay>=0?"CS+":"CS",evt.timeOfDay);
				break;
				
			case TOD_ABSOLUTE:
				sprintf(offsetBuf, "" );
				
				break;
				
			default:
				sprintf(offsetBuf, "????" );
				break;
		}
		
		length += sprintf(buff+length, "%2u: %-7s %-8s %02d:%02d%2s - " , evt.eventNum,
								evenStr[evt.event],
								offsetBuf,
								hourFormat12(when),minute(when), isAM(when)?"am":"pm");
		
		if(lastRun != 0) {
			
			bool doneToday = elapsedDays(lastRun) == elapsedDays(now);
			
			if(doneToday){
				length += sprintf(buff+length,"%-10s %02d:%02d %2s ", "Today",  hourFormat12(lastRun),minute(lastRun), isAM(lastRun)?"AM":"PM");
				
			} else
				length += sprintf(buff+length,"%d/%02d/%4d %02d:%02d %2s ",
										month(lastRun), day(lastRun), year(lastRun),
										hourFormat12(lastRun),minute(lastRun), isAM(lastRun)?"AM":"PM");
			
		}
		Serial_Printf(serial, buff);
	}
	
	if(tempSensor.hasSensor){
		float highTrigger, lowTrigger;
		SavedEvent highEvent, lowEvent;
		
		eeMgr.get_TempHighTrigger(&highTrigger,&highEvent);
		eeMgr.get_TempLowTrigger(&lowTrigger,&lowEvent);
		
		if(lowEvent != SE_NONE){
			Serial_Printf(serial, "    %-7s %8s Temp < %.2f\xA1%c" , evenStr[lowEvent],"",lowTrigger,'F');
		}
		
		if(highEvent != SE_NONE){
			Serial_Printf(serial, "    %-7s %8s Temp > %.2f\xA1%c" , evenStr[highEvent],"",highTrigger,'F');
			
		}
	}
	
	Serial_Printf(serial,"---");
}


