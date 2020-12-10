//
//  ScheduleMgr.hpp
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


#ifndef ScheduleMgr_hpp
#define ScheduleMgr_hpp

#include <Arduino.h>
#include "TimeLib.h"
  

#define MAX_STORED_EVENTS 16


#define  TIME_OF_DAY(_HOUR_,_MIN_)  (((_HOUR_ % 23) * 60 + _MIN_) & 0x7FF)
#define HOUR_FROM_TOD(_tod_) ((_tod_ & 0x7ff) / 60)
#define MIN_FROM_TOD(_tod_) ((_tod_ & 0x7ff)  % 60)
 

typedef enum {
 	TOD_INVALID = 0,
	TOD_ABSOLUTE,
	TOD_SUNRISE,
	TOD_SUNSET,
	TOD_CIVIL_SUNRISE,
	TOD_CIVIL_SUNSET,
} tod_offset_t;


 
typedef enum {
	SE_NONE = 0,
	SE_RELOAD,
	SE_DOOR_OPEN,
	SE_DOOR_CLOSE,
	
	SE_LIGHT1_ON,
	SE_LIGHT1_OFF,

	SE_RELAY4_ON,
	SE_RELAY4_OFF,
	
	EVT8,
	EVT9,
	EVT10,
	
	SE_INVALID = 0x0F
} SavedEvent;


 typedef struct  {
	 float temperature;
	 SavedEvent event;
} temperature_entry;   //  low  and high


extern char const* evenStr[];

extern "C++" {
class ScheduleMgr {
	
public:
	
	ScheduleMgr() {};
	
	bool loadSchedule();
	bool getNextEvent(SavedEvent *event, uint32_t *scheduledTime);
	
	bool peekNextEvent(SavedEvent *event, uint32_t *scheduledTime);
	
	bool reconcileEvents(SavedEvent *events, size_t inCount,  size_t *outCount );
 
	void printSchedule(Stream *serial);
	
	private:

	SavedEvent inverseEvent(SavedEvent event);

	bool tableLoaded;
	
	temperature_entry _lowTemp;
	temperature_entry _highTemp;
	
	bool _didLowTemp;
	bool _didHighemp;

};

};
#endif /* ScheduleMgr_hpp */
