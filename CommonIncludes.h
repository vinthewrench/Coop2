//
//  CommonIncludes.h
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

#ifndef WifiArtTest_h
#define WifiArtTest_h

#define DEBUG_PRINT 1

#include <Wire.h>
#include <SparkFunTMP102.h>  		// Temp Sensor
#include <SerLCD.h> 			//  http://librarymanager/All#SparkFun_SerLCD
#include "SparkFun_Qwiic_Relay.h"
#include <Arduino_JSON.h>


#include "WifiMgr.h"
#include "RTC.h"
#include "TimeSyncMgr.h"
#include "EEPPROMMgr.h"
#include "TelnetPortMgr.h"
#include "RESTportMgr.h"

#include "ScheduleMgr.h"
#include "CmdLineMgr.h"

#include "Utilities.h"
#include "StateMgr.h"
#include "door.h"
#include "buttons.h"
#include "display.h"
#include "TempSensor.h"
#include "PowerMgr.h"


extern WifiMgr			wifiMgr;
extern TimeSyncMgr 		tsMgr;
extern EEPPROMMgr 		eeMgr;
extern ScheduleMgr   	scdMgr;
extern StateMgr			stateMgr;
extern CmdLineMgr		cmdLineMgr;
extern TelnetPortMgr	telnetPortMgr;
extern RESTportMgr		restPortMgr;
 
extern Door				door;
extern Display 			display;
extern	Buttons			buttons;
extern APM3_RTC 			rtc;
extern TempSensor 		tempSensor;
extern PowerMgr 			powerMgr;

extern const char* 		versionString;

typedef enum {
	REMOTE_NONE = 0,
	REMOTE_REST,
	REMOTE_TELNET,
}remote_protocol_t;


 
extern "C" {

void debug_printf(const char *fmt, ...);

#define CMPSTR(_st1_, _st2_)  (strcasecmp(_st1_,_st2_) == 0)

#if DEBUG_PRINT
 
#define DPRINTF(...) debug_printf(__VA_ARGS__)
#else
#define DPRINTF( ...)
#endif


} // extern c


#endif /* WifiArtTest_h */
