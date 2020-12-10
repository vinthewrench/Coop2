//
//  CmdLineMgr.cpp
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

#include "CmdLineMgr.h"
#include "CommonIncludes.h"

// MARK:  - CmdLineMgr Defines

extern "C" {

const char strCmdDoor[]   	= "DOOR";
const char strCmdLight[]   	= "LIGHT";
const char strCmdRelay4[]   	= "RL4";
const char strCmdDate[]   	= "DATE";
const char strCmdTime[]   	= "TIME";
const char strCmdStatus[]   =  "STATUS";
const char strCmdSSID[]   	= "SSID";
const char strCmdPWD[]   		= "PASSWORD";
const char strCmdLOC[]   		= "LOCATION";
const char strCmdTZ[]   		= "TZ";
const char strCmdEvent[]   	= "EVENT";
const char strCmdCLEAR[]   	= "CLEAR-EEPROM";
const char strCmdUSEDST[]   	= "USE-DST";
const char strCmdNTP[]   		= "NTP";
const char strCmdSYNC[]   	= "SYNC";

const char strCmdNExtEvent[] = "NEXT";

const char strCmdRemotePort[]  = "REMOTE";
const char strCmdWifi[]   		= "WIFI";


const char strCmdHiTemp[]   	= "TEMP-LOW";
const char strCmdLowTemp[]   = "TEMP-HIGH";

const char strCmdShed[]   	= "SCHEDULE";

const char strCmdArgYES[]   	= "YES";
const char strCmdArgNO[]   	= "NO";
const char strCmdArgON[]   	= "ON";
const char strCmdArgOFF[]   	= "OFF";
const char strCmdArgAUTO[]	= "AUTO";

const char strCmdArgOpen[]   = "OPEN";
const char strCmdArgClose[]  = "CLOSE";
const char strCmdArgReload[]  = "RELOAD";
const char strCmdArgNone[]   = "NONE";

const char strCmdArgL1ON[]   = "L1-ON";
const char strCmdArgL1OFF[]  = "L1-OFF";

const char strCmdArgRELAY4ON[]   = "RL4-ON";
const char strCmdArgRELAY4OFF[]  = "RL4-OFF";
 
const char strCmdArgStart[]   = "START";
const char strCmdArgStop[]  	 = "STOP";

const char strCmdArgAPList[]   = "LIST";
const char strCmdDisplay[]  	 = "DISPLAY";

const char strCmdArgTelnet[]   = "TELNET";
const char strCmdArgREST[]   = "REST";


const char *strHELP =
"SET DOOR <OPEN, CLOSE>\n"
"SET LIGHT <ON, OFF>\n"
"SET RL4 <ON, OFF>\n"
"SET SSID  <\"ssid name\"> \n"
"SET PASSWORD  <\"password\"> \n"
"SET LOCATION [lat] [long]\n"
"SET TZ <GMT offset>\n"
"SET REMOTE <OFF, TELNET, REST>   [portnum]>\n"
"SET WIFI <ON,OFF,AUTO>\n"
"SET NTP <time server IP/DNS>\n"
"SET SYNC <seconds to resync>\n"
"SET USE-DST <YES/NO>\n"
"SET DISPLAY <OFF,RED,ORANGE,YELLOW,GREEN,BLUE,INIDGO,VIOLET,GREY,WHITE\n"
"SET EVENT [evtNum] <OPEN/CLOSE/L1-ON/L1-OFF/RL4-ON/RL4-OFF/RELOAD/NONE> [hh:mm] /<SR,SS,CR,CS mins>\n"
"SET TEMP-LOW [temp] <OPEN/CLOSE/L1-ON/L1-OFF/RL4-ON/RL4-OFF/RELOAD/NONE>\n"
"SET TEMP-HIGH [temp] <OPEN/CLOSE/L1-ON/L1-OFF/RL4-ON/RL4-OFF/RELOAD/NONE>\n"
"CLEAR-EEPROM\n"
"IC2-SCAN\n"
"SHOW <SCHEDULE, STATUS, NEXT, SSID, REMOTE>\n"
"WIFI <START, STOP, LIST, SSID>\n"
"REMOTE <LIST, CLOSE [portNum]> \n"
"RESTART\n"
"DATE\n"
"SYNC\n" ;
 
/*
 set event 0 reload 00:00
// set event 1 l1-on cr -10
 set event 1 l1-on 06:00
 set event 2 open cr
// set event 3 l1-off sr +10
 set event 3 l1-off 09:00
 set event 4 close cs
 set event 5 rl4-on 10:59
 set event 6 rl4-off 11:00
 
 set temp-low 32 rl4-on
*/
 

// MARK:  - CmdLineMgr foward declarations

void printTime(Stream* serial);
void printStatus(Stream* serial);
void printAPList(Stream* serial);
void printSSIDinfo(Stream* serial);
void printPortinfo(CmdLineMgr* cbMgr);
void printNextEvent(CmdLineMgr* cbMgr);

SavedEvent CommandToEvent(const char* str)
{
	SavedEvent evt = SE_INVALID;
	
	if(CMPSTR(str, strCmdArgNone))				evt = SE_NONE;
	else if(CMPSTR(str, strCmdArgReload)) 	evt = SE_RELOAD;
	else if(CMPSTR(str, strCmdArgOpen)) 		evt = SE_DOOR_OPEN;
	else if(CMPSTR(str, strCmdArgClose)) 	evt = SE_DOOR_CLOSE;
	else if(CMPSTR(str, strCmdArgL1ON)) 		evt = SE_LIGHT1_ON;
	else if(CMPSTR(str, strCmdArgL1OFF))		evt = SE_LIGHT1_OFF;
	else if(CMPSTR(str, strCmdArgRELAY4ON))	evt = SE_RELAY4_ON;
	else if(CMPSTR(str, strCmdArgRELAY4OFF))evt = SE_RELAY4_OFF;
	
	return evt;
}

// MARK:  - CmdLineMgr callbacks (SET)


void functSet(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	char* param = myParser->getCmdParam(1);
	char* value = myParser->getValueFromKey(param);
	
	if(CMPSTR(param, strCmdPWD) )
	{
		eeMgr.store_wifiPwd(value);
		return;
	}
	// needs a value
	else if(value != NULL){
		
		if(CMPSTR(param, strCmdSSID)) {
			eeMgr.store_ssid(value);
			return;
		}
		
		else if(CMPSTR(param, strCmdNTP)) {
			eeMgr.set_timeServerName(value);
			return;
		}
	
		else if(CMPSTR(param, strCmdSYNC)) {
	 
			unsigned long seconds = atol(value);
			if(seconds > 30) {
				eeMgr.set_timeSyncInterval(seconds);
				tsMgr.updatePrefs();
				return;
			}
			goto error;
  		}
	
		else if(CMPSTR(param, strCmdDisplay)) {
			const char *arg1 = myParser->getCmdParam(2);
	
			BLColor color = display.stringToColor(arg1);
			if(color != BLColor::invalid) {
				display.setBackLight(color);
				eeMgr.set_backlight((unsigned long) color);
				return;
			}
			goto error;
		}
		else if(CMPSTR(param, strCmdHiTemp)) {
			const char *arg1 = myParser->getCmdParam(2);
			const char *arg2 = myParser->getCmdParam(3);
			
			if(arg1 && arg2) {
				
				float temp = atof(arg1);
				SavedEvent evt = CommandToEvent(arg2);
				if(evt != SE_INVALID ){
					eeMgr.set_TempHighTrigger(temp,evt);
					scdMgr.loadSchedule();
					return;
				}
			}
		}
		
		else if(CMPSTR(param, strCmdLowTemp)) {
			const char *arg1 = myParser->getCmdParam(2);
			const char *arg2 = myParser->getCmdParam(3);
			
			if(arg1 && arg2) {
				
				float temp = atof(arg1);
				SavedEvent evt = CommandToEvent(arg2);
				if(evt != SE_INVALID ){
					eeMgr.set_TempLowTrigger(temp,evt);
					scdMgr.loadSchedule();
					return;
				}
				
			}
		}
 
		else if(CMPSTR(param, strCmdUSEDST)) {
			pref_bits_entry prefs;
			eeMgr.get_prefBits(&prefs);
			
			if(CMPSTR(value, strCmdArgYES)) {
				prefs.useDST = true;
				eeMgr.set_prefBits(prefs);
				tsMgr.updatePrefs();
				return;
			}
			else if(CMPSTR(value, strCmdArgNO) )
			{
				prefs.useDST = false;
				eeMgr.set_prefBits(prefs);
				tsMgr.updatePrefs();
				return;
			}
		}
		else if(CMPSTR(param, strCmdLight)) {
			
			if(CMPSTR(value, strCmdArgON)) {
				door.setLight1(true);
				return;
			}
			else if(CMPSTR(value, strCmdArgOFF) )
			{
				door.setLight1(false);
				return;
			}
		}
		
		else if(CMPSTR(param, strCmdWifi)) {
			
			char *arg1 = myParser->getCmdParam(2);
			if(arg1){
				
				pref_bits_entry prefs;

				if(CMPSTR(arg1, strCmdArgON)) {
					eeMgr.get_prefBits(&prefs);
	 				prefs.wifiOnStartup 	= true;
					prefs.wifiAuto		 	= true;
					eeMgr.set_prefBits(prefs);
					wifiMgr.setAPConnection(true);
					return;
				}
				else if(CMPSTR(arg1, strCmdArgOFF)) {
					eeMgr.get_prefBits(&prefs);
					prefs.wifiOnStartup 	= false;
					prefs.wifiAuto		 	= false;
					eeMgr.set_prefBits(prefs);
					wifiMgr.setAPConnection(false);
					return;
				}
				else if(CMPSTR(arg1, strCmdArgAUTO)) {
					eeMgr.get_prefBits(&prefs);
					prefs.wifiOnStartup 	= false;
					prefs.wifiAuto		 	= true;
					eeMgr.set_prefBits(prefs);
					wifiMgr.setAPConnection(false);
					return;
				}
			}
		}
	
		else if(CMPSTR(param, strCmdRemotePort)) {
			
			char *arg1 = myParser->getCmdParam(2);
			char *arg2 = myParser->getCmdParam(3);
 
			if(arg1){
				
				pref_bits_entry prefs;
				eeMgr.get_prefBits(&prefs);
				
				uint16_t portnum  = 0;
				remote_protocol_t protocol = REMOTE_NONE;
				
				if(CMPSTR(arg1, strCmdArgOFF)) {
					portnum  = 0;
				} else if(CMPSTR(arg1, strCmdArgTelnet)) {
					protocol = REMOTE_TELNET;
					portnum  = 23;
				}
				else if(CMPSTR(arg1, strCmdArgREST)) {
					portnum  = 80;
					protocol = REMOTE_REST;
				}
				else goto error;
			
				if(arg2){
					portnum = atoi(arg1);
				};
	
				eeMgr.set_remoteTermPort(portnum);

				prefs.remoteTermOnStartup = 0;
				prefs.remoteRESTOnStartup = 0;

				switch(protocol){
					case REMOTE_REST:
						prefs.remoteRESTOnStartup = 1;
						break;
						
					case REMOTE_TELNET:
						prefs.remoteTermOnStartup = 1;
						break;
						
					case REMOTE_NONE:
						break;
				}
				
 				eeMgr.set_prefBits(prefs);
 
				if(portnum == 0){
					telnetPortMgr.stop();
					restPortMgr.stop();
				}
				else {
					// if the network is not running then start it and it will
					// start the telnetPortMgr
 
					if(!wifiMgr.hasInterNet){
						wifiMgr.setAPConnection(true);
					}
					// network is running -- start the portMgtr
					else {
						if(protocol == REMOTE_TELNET){
							restPortMgr.stop();
							telnetPortMgr.begin(portnum);
						}
						else if(protocol == REMOTE_REST){
							telnetPortMgr.stop();
							restPortMgr.begin(portnum);
						}

					}
				}
				return;
			}
		}
		else if(CMPSTR(param, strCmdRelay4)) {
				
				if(CMPSTR(value, strCmdArgON)) {
					door.setRelay4(true);
					return;
				}
				else if(CMPSTR(value, strCmdArgOFF) )
				{
					door.setRelay4(false);
					return;
				}
			}
		
		else if(CMPSTR(param, strCmdDoor)) {
			if(CMPSTR(value, strCmdArgOpen )) {
				stateMgr.receive_event( EV_OPEN);
				return;
				
			}
			else if(CMPSTR(value, strCmdArgClose) )
			{
				stateMgr.receive_event( EV_CLOSE);
				return;
			}
		}
		else if(CMPSTR(param, strCmdLOC)) {
			
			char *arg1 = myParser->getCmdParam(2);
			char *arg2 = myParser->getCmdParam(3);
			if(arg1 && arg2) {
				double Latitude = atof(arg1);
				double Longitude = atof(arg2);
				
				if(Longitude >= -180.0 && Longitude <= 180.0
					&& Latitude >= -90.0 && Latitude <= 90.0 ) {
					int    extOffset;
					eeMgr.get_timezone(&extOffset,NULL,NULL);
					eeMgr.set_timezone(extOffset,Latitude,Longitude);
					tsMgr.init();
					return;
				}
			}
		}
		
		else if(CMPSTR(param, strCmdTZ)) {
			char *arg1 = myParser->getCmdParam(2);
			if(arg1) {
				int tzOffset = atoi(arg1);
				if(tzOffset >= -12 && tzOffset <= 14){
					double exLongitude, exlatitude;
					
					eeMgr.get_timezone(NULL,&exlatitude,&exLongitude);
					eeMgr.set_timezone(tzOffset,exlatitude,exLongitude);
					tsMgr.init();
					return;
				}
			}
		}
		
		else if(CMPSTR(param, strCmdEvent)) {
			char *arg1 = myParser->getCmdParam(2);
			char *arg2 = myParser->getCmdParam(3);
			char *arg3 = myParser->getCmdParam(4);
			char *arg4 = myParser->getCmdParam(5);
			
			if(arg1 && arg2 ){
				int evtNum = atoi(arg1);
				SavedEvent	evt = SE_INVALID;
				
				tod_offset_t timebase = TOD_INVALID;
				int hour,min;
				
				int16_t tod = 0;
				
				evt = CommandToEvent(arg2);
		
				if(evt != SE_INVALID || evt != SE_NONE ){
					
					int res = sscanf(arg3, "%2d:%2d",&hour,&min);
					if(res == 2){
						timebase = TOD_ABSOLUTE;
						tod = TIME_OF_DAY(hour,min);
					} else {
						if(CMPSTR(arg3, "SR")) timebase = TOD_SUNRISE;
						else if(CMPSTR(arg3, "SS")) timebase = TOD_SUNSET;
						else if(CMPSTR(arg3, "CS")) timebase = TOD_CIVIL_SUNSET;
						else if(CMPSTR(arg3, "CR")) timebase = TOD_CIVIL_SUNRISE;
						
						if( timebase != TOD_INVALID && arg4) {
							tod = atoi(arg4);
						}
					}
				}
	 
				if(evt != SE_INVALID
					&& ( evtNum >= 0 && evtNum < MAX_STORED_EVENTS )
					&& ( timebase != TOD_INVALID || evt == SE_NONE))
				{
					eeMgr.store_event(evtNum, evt, timebase, tod);
					scdMgr.loadSchedule();
					return;
				}
				
			}
			
		}
		
	}

	error:
		Serial_Printf(serial, "ERROR SET %s ", param);
}

// MARK:  - CmdLineMgr callbacks (STATUS)

void printStatus(Stream* serial) {
	
	time_t  date =  tsMgr.timeNow();
	time_t  sunset, sunrise, civilSunRise, civilSunSet;
	
	int    gmtOffset;
	double longitude;
	double latitude;
	
	unsigned long  maxTimeSyncInterval;
	char*  timeServerDNS = NULL;
	
	char  ssid[32] = {0};
	
	bool hasDoor = door.hasDoor;
	
	uint16_t portnum;

	pref_bits_entry prefs;
	
	sunset = tsMgr.sunSet();
	sunrise = tsMgr.sunRise();
	civilSunRise = tsMgr.civilSunRise();
	civilSunSet = tsMgr.civilSunSet();
	
	state_t currentState =  stateMgr.currentState;
	
	eeMgr.get_timezone(&gmtOffset,&latitude,&longitude);
	maxTimeSyncInterval = eeMgr.get_timeSyncInterval();
	eeMgr.get_timeServerName(&timeServerDNS);

	eeMgr.get_ssid(ssid);
	eeMgr.get_prefBits(&prefs);
	eeMgr.get_remoteTermPort(&portnum);

	time_t lastSync  = eeMgr.get_lastTimeSync();
	
	unsigned long upTime = tsMgr.upTime();
 
	Serial_Printf( serial, "%10s %3s %s %d %4d %d:%02d:%02d%s %s" ,  "TIME:",
					  dowStr(date), monStr(date), day(date), year(date),
					  hourFormat12(date), minute(date), second(date), isAM(date)?"am":"pm",
					  tsMgr.isDST(date)?"DST":"ST");
	
	if(lastSync != 0){
		Serial_Printf( serial, "%10s %3s %s %d %4d %d:%02d:%02d%s %s" ,  "SYNC:",
						  dowStr(lastSync), monStr(lastSync), day(lastSync), year(lastSync),
						  hourFormat12(lastSync), minute(lastSync), second(lastSync), isAM(lastSync)?"am":"pm",
						  tsMgr.isDST(lastSync)?"DST":"ST");
		
	}
	else
	{
		Serial_Printf( serial, "%10s %s " ,  "SYNC:", "NEVER");
	}
	
	if(upTime != 0) {
		
		tmElements_t tm;
		breakDuration(upTime, tm);
		
		if(tm.Day > 0){
			Serial_Printf( serial, "%10s %d %s, %01d:%02d:%02d hours" ,  "UpTime:",
							  tm.Day, (tm.Day>1?"Days":"Day"),
							  tm.Hour, tm.Minute, tm.Second);
		}
		else {
			Serial_Printf( serial, "%10s %01d:%02d:%02d hours" ,  "UpTime:",
							  tm.Hour, tm.Minute, tm.Second);
		}
	}
	
	Serial_Printf( serial, "%10s %d" ,  "GMT:", gmtOffset);
	Serial_Printf( serial, "%10s (%f, %f)" , "LAT/LONG:", latitude,longitude);
	
	Serial_Printf(serial, "%10s %s", "Use DST:", prefs.useDST?"YES":"NO");
	
	Serial_Printf(serial, "%10s %02d:%02d:%02d %s - %02d:%02d:%02d %s", "SUNRISE:",
					  hourFormat12(civilSunRise), minute(civilSunRise), second(civilSunRise),isAM(civilSunRise)?"am":"pm",
					  hourFormat12(sunrise), minute(sunrise),second(sunrise),isAM(sunrise)?"am":"pm"  );
	
	Serial_Printf(serial, "%10s %02d:%02d:%02d %s - %02d:%02d:%02d %s ", "SUNSET:",
					  hourFormat12(sunset), minute(sunset), second(sunset),isAM(sunset)?"am":"pm",
					  hourFormat12(civilSunSet), minute(civilSunSet),second(civilSunSet),isAM(civilSunSet)?"am":"pm");
	
	{
		tmElements_t tm;
		breakDuration(maxTimeSyncInterval, tm);
 
		if(tm.Day > 0){
			Serial_Printf( serial, "%10s %d %s, %01d:%02d:%02d hours" ,  "TIME SYNC:",
							  tm.Day, (tm.Day>1?"Days":"Day"),
							  tm.Hour, tm.Minute, tm.Second);
		}
	 	else {
			Serial_Printf( serial, "%10s %01d:%02d:%02d hours" ,  "TIME SYNC:",
							 tm.Hour, tm.Minute, tm.Second);
		}
	}
	
	Serial_Printf( serial, "%10s %s" ,  "NTP:", timeServerDNS);

	if(!wifiMgr.hasInterNet){
		if(prefs.wifiAuto){
			Serial_Printf(serial, "%10s %s", "WIFI:", "AUTO-CONNECT");
		}
		else {
			Serial_Printf(serial, "%10s %s", "WIFI:",
							  prefs.wifiOnStartup?"NOT CONNECTED":"OFF");
		}
	}
	else
	{
		bool isConnected = wifiMgr.isConnectedToAP();
		
		if(isConnected ){
			
			const char *strIP = wifiMgr.ipAddr();
			const char *strMAC = wifiMgr.macAddr();
			
			ap_entry  apInfo;
			if(wifiMgr.getCurrentAPInfo(&apInfo)) {
				
				Serial_Printf(serial, "%10s \"%s\" %17s rssi:%d", "WIFI:",
								  apInfo.ssid ,
								  apInfo.mac,
								  apInfo.rssi);
			}
			
			Serial_Printf(serial, "%10s %s", "IP:",strIP?strIP:"???");
			Serial_Printf(serial, "%10s %s", "MAC:",strMAC?strMAC:"???");
			
		}
		else {
			Serial_Printf(serial, "%10s %s", "WIFI:",
							  isConnected?"Connected":"Not-Connected");
		}
	}
	
	Serial_Printf(serial, "%10s %.*s", "SSID:", 32, ssid);
	
	if(portnum == 0){
		Serial_Printf(serial, "%10s %s", "REMOTE:", "OFF");
	} else {
		
		if(prefs.remoteTermOnStartup){
			Serial_Printf(serial, "%10s %s %u", "REMOTE:", "TELNET", portnum );
		}else if(prefs.remoteRESTOnStartup){
			Serial_Printf(serial, "%10s %s %u", "REMOTE:", "REST", portnum );
		}
	}
	 
	if(hasDoor){
		Serial_Printf(serial, "%10s %s", "DOOR:", stateMgr.stateText(currentState));
		
		Serial_Printf(serial, "%10s %s", "LIGHT:", door.light1()?"ON":"OFF");
		
		Serial_Printf(serial, "%10s %s", "RELAY4:", door.relay4()?"ON":"OFF");
	 
	}
  else {
	  Serial_Printf(serial, "%10s %s", "DOOR:", "NOT CONNECTED");
	  
	  Serial_Printf(serial, "%10s %s", "LIGHT:",  "NOT CONNECTED");
	  
	  Serial_Printf(serial, "%10s %s", "RELAY4:",  "NOT CONNECTED");
	}

	bool hasRed = buttons.isConnected(BUTTON_RED);
	bool hasGreen = buttons.isConnected(BUTTON_GREEN);
	
	if(hasRed && hasGreen)
		Serial_Printf(serial, "%10s %s", "BUTTONS:", "CONNECTED");
	else if(hasRed && !hasGreen)
		Serial_Printf(serial, "%10s %s", "BUTTONS:", "NO GREEN");
	else if(!hasRed && hasGreen)
		Serial_Printf(serial, "%10s %s", "BUTTONS:", "NO RED");
	else if(!hasRed && !hasGreen)
		Serial_Printf(serial, "%10s %s", "BUTTONS:", "NOT CONNECTED");
	
	if(tempSensor.hasSensor){
		float temperature = tempSensor.readTempF();
		float highTrigger, lowTrigger;
		SavedEvent highEvent, lowEvent;
		
		eeMgr.get_TempHighTrigger(&highTrigger,&highEvent);
		eeMgr.get_TempLowTrigger(&lowTrigger,&lowEvent);
		 
		char buff[80] = {0};
		int offset =  0;
		
		if(highEvent != SE_NONE){
			offset += sprintf(buff, "HIGH = (%.2f\xA1%c  %s) " ,  highTrigger,'F' , evenStr[highEvent]);
			buff[offset] = ' ';
		}
	
		if(lowEvent != SE_NONE){
			offset += sprintf(buff +offset , "LOW = (%.2f\xA1%c  %s)" , lowTrigger,'F' , evenStr[lowEvent]);
		}
		
		Serial_Printf(serial, "%10s %.2f\xA1%c %s", "TEMP:",   temperature,'F' ,buff);
 
	}
	else {
		Serial_Printf(serial, "%10s %s", "TEMP:", "NOT INSTALLED");
	}
	
	if(powerMgr.hasSensor()){
		char buff[80] = {0};
		int offset =  0;
	
		float  voltage = powerMgr.voltage();
		float  current = 	powerMgr.average_current_mA();
	
		offset += sprintf(buff +offset , "%2.2fv %2.2fmA",voltage, current );
		Serial_Printf(serial, "%10s %s", "Power:",  buff);
	}
	else {
		Serial_Printf(serial, "%10s %s", "Battery:",   "NO SENSOR");
	}
	
	Serial_Printf(serial, "%10s %s", "AC Power:",
					  powerMgr.hasACpower()?"ON":"OFF");
 
	Serial_Printf(serial, "%10s %s", "ENCLOSURE:",   door.enclosureDoorClosed()?"SHUT":"OPEN");
 
	if(timeServerDNS){
		free(timeServerDNS);
	}
}


// MARK:  - CmdLineMgr callbacks

void functWifi(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	pref_bits_entry prefs;

	char* param = myParser->getCmdParam(1);
	if(param != NULL) {
		if(CMPSTR(param, strCmdArgStart)) {
			eeMgr.get_prefBits(&prefs);
			prefs.wifiOnStartup = 1;
			eeMgr.set_prefBits(prefs);
			wifiMgr.setAPConnection(true);
			return;
		}
		else if(CMPSTR(param, strCmdArgStop)) {
			eeMgr.get_prefBits(&prefs);
			prefs.wifiOnStartup = 0;
			eeMgr.set_prefBits(prefs);
			wifiMgr.setAPConnection(false);
			return;
		}
		else if(CMPSTR(param, strCmdArgAPList)) {
			printAPList(serial);
			return;
		}
		else if(CMPSTR(param, strCmdSSID)) {
			printSSIDinfo(serial);
			return;
		}
	}
	
	Serial_Printf(serial, "ERROR WIFI %s ", myParser->getParamCount());
}

void funcZERO(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
 	eeMgr.clear_all();
	scdMgr.loadSchedule();
	scdMgr.printSchedule(serial);
};

void funcIC2(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
 
	
	Serial_Printf(serial, "Scanning I2C... ");
	
	uint8_t error, address;
	int nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		
		if (error == 0)
		{
			const char* description = IC2DeviceName(address);
			
			Serial_Printf(serial,"%02x: %s",address, description?description:"???" );
			nDevices++;
		}
		//    else if (error == 4)
		//    {
		//      Serial.print("Unknown error at address 0x");
		//      if (address < 16)
		//        Serial.print("0");
		//      Serial.println(address, HEX);
		//    }
	}
	if (nDevices == 0)
		Serial_Printf(serial,"No I2C devices found\n");
	else
		Serial_Printf(serial,"----\n");
};



void funcPort(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	char* param = myParser->getCmdParam(1);
	const char *arg1 = myParser->getCmdParam(2);
	
	if(param != NULL) {
		if(CMPSTR(param, strCmdArgAPList)) {
			printPortinfo(cbMgr);
			return;
		}
		else if(CMPSTR(param, strCmdArgClose)) {
			if(arg1) {
				uint16_t linkid = atoi(arg1);
				
				if((linkid >= 0) && (linkid < MAX_WIFI_LINKS)) {
					cipStatus_entry cip;
					if(wifiMgr.getConnectionInfo(linkid, &cip)){
						
						if(cip.isServer && cip.localPortNum != 0){
				 
							if(!telnetPortMgr.closeRemoteConnection(linkid)){
								Serial_Printf(serial, " CLOSE PORT  %u fail", linkid);
							}
							return;
						}
					}
				}
			}
		}
	}
	
	Serial_Printf(serial, "ERROR PORT %s ", param);
	
}

void funcShow(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	char* param = myParser->getCmdParam(1);
	if(param != NULL) {
		if(CMPSTR(param, strCmdShed)) {
			scdMgr.printSchedule(serial);
			return;
		}
		else if(CMPSTR(param, strCmdStatus)) {
			printStatus(serial);
			return;
		}
		else if(CMPSTR(param, strCmdSSID) )
		{
			printSSIDinfo(serial);
			return;
		}
		else if(CMPSTR(param, strCmdRemotePort) )
		{
			printPortinfo(cbMgr);
			return;
		}
		else if(CMPSTR(param, strCmdNExtEvent) )
		{
			printNextEvent(cbMgr);
			return;
		}
 
	}
	
	Serial_Printf(serial, "ERROR SHOW %s ", param);
	
}

void funcTime(void* arg, CmdParser *myParser)
{
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	printTime(serial);
	
}

void funcHelp(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	Serial_Printf(serial, "Commands are:\n%s\n", strHELP);
}


static const char* connTypeStr(CIPC_t conTyp){
	
	const char* val = "???";
	
	switch (conTyp) {
		case CIPC_UDP:
			val = "UPD";
			break;
			
		case CIPC_TCP:
			val = "TCP";
			break;

		case CIPC_SSL:
			val = "SSL";
			break;

		default:
			break;
	}
	
	return val;
}

void printNextEvent(CmdLineMgr* cbMgr){
	
	Stream* serial = cbMgr->_serialOut;

	SavedEvent  	event;
	uint32_t 		scheduledTime;
	
	bool hasEvent = scdMgr.peekNextEvent(&event, &scheduledTime);
	if(hasEvent) {
		
		Serial_Printf(serial, "  %-7s %02d:%02d %2s",evenStr[event],
						  hourFormat12(scheduledTime),minute(scheduledTime), isAM(scheduledTime)?"AM":"PM");
	}
	else
	{
		Serial_Printf(serial, "No more events scheduled today");
	}
	
}

void printPortinfo(CmdLineMgr* cbMgr) {
	
	Stream* serial = cbMgr->_serialOut;

	cipStatus_entry links[MAX_WIFI_LINKS];
 	bzero(links, sizeof(cipStatus_entry) * MAX_WIFI_LINKS );

	int count = 0;
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		wifiMgr.getConnectionInfo(idx, &links[idx]);
	}
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		
		cipStatus_entry  cpd = links[idx];
		if(cpd.localPortNum == 0) continue;
		
		count++;
 		Serial_Printf(serial, " %u: %s %c %3s/%-3u %s %d  ",
		
						  idx, cbMgr->_link_id == idx?"->":"  ",
						  cpd.isServer == 1?'S':'C',   connTypeStr(cpd.connType),   cpd.localPortNum,
 						  cpd.ipaddr,  cpd.remotePortNum );
	}
	
	if(count == 0){
		Serial_Printf(serial, " No remote connections");
	}
}



void funcStatus(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
 	printStatus(serial);
}
void funcShowSchedule(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	scdMgr.printSchedule(serial);
}

void funcRestart(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	 
	Serial_Printf(serial, " RESTART...");
	
	// force dog to bite
	am_hal_wdt_start();
	while(true) delay(1);
}

 

void printAPList(Stream* serial){
	
	bool wasWifiRunning = wifiMgr.isRunning();
	
	if(!wasWifiRunning){
		if(!wifiMgr.setPower(true)){
			Serial_Printf(serial, "WIFI failed to power on");
		}
 	}
 
	ap_entry* apTable = NULL;
	size_t ap_count = 0;
	
	if(wifiMgr.getAPList(false ,&apTable, &ap_count)) {
		
		Serial_Printf(serial, "-- found %d --", ap_count);
		// quick scan for formatting
		size_t maxspace = 0;
		for(size_t idx = 0; idx < ap_count; idx++){
			ap_entry* ap = &apTable[idx];
			size_t len = strlen(ap->ssid);
			if(len>maxspace)maxspace = len;
		}
		// print out the list
		for(size_t idx = 0; idx < ap_count; idx++){
			ap_entry* ap = &apTable[idx];
			
			// magic formatting -  we need spaces
			char ssid[32];
			memset(ssid,' ',32);
			size_t len = strnlen(ap->ssid, 32);
			memcpy(ssid, ap->ssid,len);
			
			Serial_Printf(serial, "%2d %s %.*s %17s %d",
							  idx,
							  (ap->ecn == 0?"o":" "),
							  maxspace, ssid,
							  ap->mac,
							  ap->rssi);
		}
	}
	
	if(apTable)
		free(apTable);
	
	// power the device down if we found it that way
	if(!wasWifiRunning){
		wifiMgr.setPower(false);
	}
}

void printSSIDinfo(Stream* serial) {
	
	char  ssid[32] = {0};
	char  wifiPwd[64] = {0};
	
	eeMgr.get_ssid(ssid);
	eeMgr.get_wifiPwd(wifiPwd);
	
	Serial_Printf(serial, "SSID = %.*s", 32, ssid);
	if(wifiPwd[0] != '\0'){
		Serial_Printf(serial, "PASSWORD = %.*s", 64, wifiPwd);
	}
}




void printTime(Stream* serial) {
	
	time_t  date =  tsMgr.timeNow();
	
	Serial_Printf( serial, "%3s %s %d %4d %d:%02d:%02d%s %s" ,
					  dowStr(date), monStr(date), day(date), year(date),
					  hourFormat12(date), minute(date), second(date), isAM(date)?"am":"pm",
					  tsMgr.isDST(date)?"DST":"ST");
	
}


void functSync(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	Serial_Printf(serial, " START NTP SYNC..");
	tsMgr.syncClockIfNeeded(true);
}

void funcErr(void* arg, CmdBufferObject *buffer) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	Serial_Printf(serial, "ERR: %s\n",buffer->getStringFromBuffer());
}



// this gets called when we are done processing a command.
void funcComplete(void* arg, CmdParser *myParser) {
	
	CmdLineMgr* cbMgr = (CmdLineMgr*) arg;
	Stream* serial = cbMgr->_serialOut;
	
	cbMgr->cmdBuffer.displayPrompt(serial);

}

} /* extern C */


// MARK:  - CmdLineMgr

CmdLineMgr::CmdLineMgr()
{
	_out = new LoopbackStream(1024);

	cmdCallback.setCallBackArg(this);
	
	cmdCallback.addCmd("HELP", &funcHelp);
	cmdCallback.addCmd("SET", &functSet);
	cmdCallback.addCmd("SHOW", &funcShow);
	cmdCallback.addCmd(strCmdSYNC, &functSync);
	cmdCallback.addCmd(strCmdWifi, &functWifi);
	cmdCallback.addCmd("DATE", &funcTime);
	cmdCallback.addCmd("IC2-SCAN", &funcIC2);
	cmdCallback.addCmd("RESTART", &funcRestart);

	cmdCallback.addCmd(strCmdRemotePort, 	&funcPort);
	cmdCallback.addCmd(strCmdStatus, &funcStatus);
	cmdCallback.addCmd(strCmdShed, &funcShowSchedule);

	cmdCallback.addCmd(strCmdCLEAR, &funcZERO);
 
	cmdCallback.addCompletionHandler( &funcComplete);
	cmdCallback.addErrorHandler( &funcErr);
}


void CmdLineMgr::begin(Stream *input ,
							  Stream *output,
							  uint8_t link_id)
{
	_serialIn 	= input;
	_serialOut 	= output;
	_link_id 	= link_id;
 	
	
	if(_serialIn == _serialOut){
		cmdBuffer.setEcho(true);
 	}
  else {
	  cmdBuffer.setEcho(false);
  }
	
	isSetup = true;
	
	cmdBuffer.clear();
	
	Serial_Printf(_serialOut, "ChickenCoop Shell:");
 
	time_status tstatus = tsMgr.timeSyncStatus();

	if(tstatus == TIME_NOT_SET){
		Serial_Printf(_serialOut, " TIME IS NOT SET! Please do a SYNC");
	}
		
	cmdBuffer.displayPrompt(_serialOut);
}


void CmdLineMgr::stop(){

//	Serial_Printf(&Serial, "CmdLineMgr::stop ");
 
	isSetup = false;
	
 	cmdBuffer.clear();

 	_out->clear();
	
   delete _out;
 	_out = NULL;
	_serialOut = NULL;
	_serialIn 	= NULL;
	
 }


void CmdLineMgr::loop()
{
	if(!isSetup)
		return;
	
	cmdCallback.updateCmdProcessing(&cmdParser, &cmdBuffer, _serialIn);

 if(!this->isBusy() && (_out->available() > 0))
 {
		 _serialOut->println();
		 while(_out->available() > 0){
			 
			 size_t dataLen = _out->available();
			 char data[128];
			 
			 if(dataLen > sizeof(data))
				 dataLen = sizeof(data);
			 
			 for(int i = 0 ; i < dataLen ;i++){
				 data[i] =  _out->read();
			 }

			 _serialOut->write(data,dataLen);
		 }
	 
 		funcComplete(this, &cmdParser);
 	}
}


bool CmdLineMgr::isBusy()
{
	return  cmdBuffer.hasDataInBuffer();
}


void CmdLineMgr::print(const char *fmt, ...){

	char buff[1024];
	
	va_list args;
	va_start(args, fmt);
	
	size_t len = 0;
	
	len += snprintf(buff+len, sizeof(buff)-len, " ");
	len += vsnprintf(buff+1, sizeof(buff) - 2, fmt, args);
	len += snprintf(buff+len, sizeof(buff)-len, "\n");
 
 	_out->write((const uint8_t*)buff, len);
 
	telnetPortMgr.broadcast(buff);
	
	va_end(args);
}
