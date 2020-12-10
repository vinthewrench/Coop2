//
//  RESTportMgr.cpp
//  Coop2
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

#include "RESTportMgr.h"
#include "CommonIncludes.h"
#include "yuarel.h"

static const char* hdr404 = "HTTP/1.1 404 Not Found\r\n"
									"Content-Type: text/html\r\n"
									"Content-Length: 11\r\n"
									"Connection: close\r\n"
									"\r\n"
									"Not found:\r\n";
 
static const char* hdr400 = "HTTP/1.1 400 Bad Request\r\n"
"Connection: close\r\n"
	 "\r\n";

static const char* hdr204 =  "HTTP/1.1 204 No Content\r\n"
"Connection: keep-alive\r\n"
	 "\r\n";

static const char* hdr200JSON =  "HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
						"Content-Length: %u\r\n"
						"Connection: close\r\n"
						"\r\n";

int on_message_begin(http_parser* _parser) {
 
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

	if(arg->_url){
		free(arg->_url);
		arg->_url = NULL;
	}
	
	if(arg->_body){
		free(arg->_body);
		arg->_body = NULL;
	}


//	Serial_Printf(&Serial,"\n***MESSAGE BEGIN***");
  return 0;
}

int on_headers_complete(http_parser* _parser) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

//	Serial_Printf(&Serial,"Method: %s", http_method_str((enum http_method )_parser->method));
//
//	Serial_Printf(&Serial,"\n***HEADERS COMPLETE***");
  return 0;
}

int on_message_complete(http_parser* _parser) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;
	RESTportMgr* rp = (RESTportMgr*) arg->_rp;
	
	//	Serial_Printf(&Serial,"\n***MESSAGE COMPLETE***");
	
	struct yuarel url;
	
	String reply;
	
	yuarel_parse(&url, arg->_url);
	
//	Serial_Printf(&Serial,"%u %s",_parser->method, url.path);
	
	switch  (_parser->method) {
			
		case HTTP_GET:
			if(strcmp(url.path, "status") == 0){
				reply = rp->statusJSON();
			} else 	if(strcmp(url.path, "status/min") == 0){
				reply = rp->minstatusJSON();
			} else {
				reply = hdr404;
			}
			break;
			
		case HTTP_PUT:
			if(strcmp(url.path, "set") == 0){
				reply = rp->setJSON(arg);
				
			} else if(strcmp(url.path, "sync") == 0){
				reply = rp->doSync();
			} else {
				reply = hdr404;
			}
			break;
			
		default:
			reply = hdr404;
			
			break;
	}
	
	const char* str = reply.c_str();
	
	if(arg->_networkOut != NULL){
		arg->_networkOut->write((const uint8_t*) str, strlen(str));
//		arg->_networkOut->done = true;
	}
	
	if(arg->_url){
		free(arg->_url);
		arg->_url = NULL;
	}
	
	if(arg->_body){
		free(arg->_body);
		arg->_body = NULL;
	}
	
	return 0;
}

int on_url(http_parser* _parser, const char* at, size_t length) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

	arg->_url = strndup(at,length);
	
 //		Serial_Printf(&Serial,"Url: %.*s", (int)length, at);
  return 0;
}

int on_header_field(http_parser* _parser, const char* at, size_t length) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

// 	Serial_Printf(&Serial,"Header field: %.*s", (int)length, at);
  return 0;
}

int on_header_value(http_parser* _parser, const char* at, size_t length) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

// 	Serial_Printf(&Serial,"Header value: %.*s", (int)length, at);
  return 0;
}

int on_body(http_parser* _parser, const char* at, size_t length) {
	restPort_arg_t * arg = 	(restPort_arg_t*)_parser->data;

	if(!arg->_body) {
		arg->_body = strndup(at,length);
	} else {
		arg->_body = (char*) realloc(arg->_body, strlen(arg->_body) + length + 1);
		arg->_body = strncat(arg->_body,at,length);
 	}
  return 0;
}


//
// SERVER RECV [0] len:194, 10.10.0.13 54233
//		0: 4745 5420 2F73 7461   GET./sta
//		8: 7475 7320 4854 5450   tus.HTTP
//	  16: 2F31 2E31 0D0A 486F   /1.1..Ho
//	  24: 7374 3A20 3130 2E31   st:.10.1
//	  32: 302E 302E 3732 0D0A   0.0.72..
//	  40: 4163 6365 7074 3A20   Accept:.
//	  48: 2A2F 2A0D 0A41 6363   */*..Acc
//	  56: 6570 742D 4C61 6E67   ept-Lang
//	  64: 7561 6765 3A20 656E   uage:.en
//	  72: 2D75 730D 0A43 6F6E   -us..Con
//	  80: 6E65 6374 696F 6E3A   nection:
//	  88: 206B 6565 702D 616C   .keep-al
//	  96: 6976 650D 0A41 6363   ive..Acc
//	 104: 6570 742D 456E 636F   ept-Enco
//	 112: 6469 6E67 3A20 677A   ding:.gz
//	 120: 6970 2C20 6465 666C   ip,.defl
//	 128: 6174 650D 0A55 7365   ate..Use
//	 136: 722D 4167 656E 743A   r-Agent:
//	 144: 2063 6869 636B 656E   .chicken
//	 152: 2532 3063 6F6F 702F   %20coop/
//	 160: 3120 4346 4E65 7477   1.CFNetw
//	 168: 6F72 6B2F 3132 3036   ork/1206
//	 176: 2044 6172 7769 6E2F   .Darwin/
//	 184: 3139 2E36 2E30 0D0A   19.6.0..
//	 192: 0D0A                     ..
// Url: /status
// Header field: Host
// Header value: 10.10.0.72
// Header field: Accept
// Header value: */*
// Header field: Accept-Language
// Header value: en-us
// Header field: Connection
// Header value: keep-alive
// Header field: Accept-Encoding
// Header value: gzip, defl
// Header value: ate
// Header field: User-Agent
// Header value: chicken%20coop/1 CFNetwork/1206 Darwin/19.6.0

//SERVER RECV [0] len:243, 10.10.0.13 54249
//	  0: 5055 5420 2F73 6574   PUT./set
//	  8: 2048 5454 502F 312E   .HTTP/1.
//	 16: 310D 0A48 6F73 743A   1..Host:
//	 24: 2031 302E 3130 2E30   .10.10.0
//	 32: 2E37 320D 0A43 6F6E   .72..Con
//	 40: 7465 6E74 2D54 7970   tent-Typ
//	 48: 653A 2061 7070 6C69   e:.appli
//	 56: 6361 7469 6F6E 2F6A   cation/j
//	 64: 736F 6E0D 0A43 6F6E   son..Con
//	 72: 6E65 6374 696F 6E3A   nection:
//	 80: 206B 6565 702D 616C   .keep-al
//	 88: 6976 650D 0A41 6363   ive..Acc
//	 96: 6570 743A 202A 2F2A   ept:.*/*
//	104: 0D0A 5573 6572 2D41   ..User-A
//	112: 6765 6E74 3A20 6368   gent:.ch
//	120: 6963 6B65 6E25 3230   icken%20
//	128: 636F 6F70 2F31 2043   coop/1.C
//	136: 464E 6574 776F 726B   FNetwork
//	144: 2F31 3230 3620 4461   /1206.Da
//	152: 7277 696E 2F31 392E   rwin/19.
//	160: 362E 300D 0A43 6F6E   6.0..Con
//	168: 7465 6E74 2D4C 656E   tent-Len
//	176: 6774 683A 2031 350D   gth:.15.
//	184: 0A41 6363 6570 742D   .Accept-
//	192: 4C61 6E67 7561 6765   Language
//	200: 3A20 656E 2D75 730D   :.en-us.
//	208: 0A41 6363 6570 742D   .Accept-
//	216: 456E 636F 6469 6E67   Encoding
//	224: 3A20 677A 6970 2C20   :.gzip,.
//	232: 6465 666C 6174 650D   deflate.
//	240: 0A0D 0A                 ...
//SERVER RECV [0] len:15, 10.10.0.13 54249
//	  0: 7B22 6C69 6768 7422   {"light"
//	  8: 3A66 616C 7365 7D     :false}
//Url: /set
//Header field: Host
//Header value: 10.10.0.72
//Header field: Content-Type
//Header value: application/json
//Header field: Connection
//Header value: keep-alive
//Header field: Accept
//Header value: */*
//Header field: User-Agent
//Header value: chicken%20
//Header value: coop/1 CFNetwork/1206 Darwin/19.6.0
//Header field: Content-Length
//Header value: 15
//Header field: Accept-Language
//Header value: en-us
//Header field: Accept-Encoding
//Header value: gzip, deflate
//Set Light OFF |¯«.|

/*
 https://github.com/nodejs/http-parser
 
 In case you parse HTTP message in chunks (i.e. read() request line from socket, parse, read half headers, parse, etc) your data callbacks may be called more than once. http_parser guarantees that data pointer is only valid for the lifetime of callback. You can also read() into a heap allocated buffer to avoid copying memory around if this fits your application.

 Reading headers may be a tricky task if you read/parse headers partially. Basically, you need to remember whether last header callback was field or value and apply the following logic:

 (on_header_field and on_header_value shortened to on_h_*)
  ------------------------ ------------ --------------------------------------------
 | State (prev. callback) | Callback   | Description/action                         |
  ------------------------ ------------ --------------------------------------------
 | nothing (first call)   | on_h_field | Allocate new buffer and copy callback data |
 |                        |            | into it                                    |
  ------------------------ ------------ --------------------------------------------
 | value                  | on_h_field | New header started.                        |
 |                        |            | Copy current name,value buffers to headers |
 |                        |            | list and allocate new buffer for new name  |
  ------------------------ ------------ --------------------------------------------
 | field                  | on_h_field | Previous name continues. Reallocate name   |
 |                        |            | buffer and append callback data to it      |
  ------------------------ ------------ --------------------------------------------
 | field                  | on_h_value | Value for current header started. Allocate |
 |                        |            | new buffer and copy callback data to it    |
  ------------------------ ------------ --------------------------------------------
 | value                  | on_h_value | Value continues. Reallocate value buffer   |
 |                        |            | and append callback data to it             |
  ------------------------ ------------ --------------------------------------------
 */
//



void httpServerCallback(uint8_t 		link_id,
						  void*		arg,
						  int  		status,
						  char			ipaddr[16],
						  int 			portNum,
					const uint8_t* 	data,
						  size_t		dataLen){
	
	RESTportMgr* rp = (RESTportMgr*) arg;
 
	if(!rp->_running)
		return;

	if(status == WFCF_CLOSED){
	
  //		Serial_Printf(&Serial, "SERVER CLOSED [%d] ",link_id );

		rp->shutdownLink(link_id);
		
//		restPort_arg_t * arg =  &rp->_rpArg[link_id];
//		if(arg->_url){
//			free(arg->_url);
//			arg->_url = NULL;
//		}
//
//		if(arg->_body){
//			free(arg->_body);
//			arg->_body = NULL;
//		}
//
//		if(arg->_networkOut != NULL){
//			delete arg->_networkOut;
//			arg->_networkOut = NULL;
//		}
//
//		if(arg->_networkIn != NULL){
//			delete arg->_networkIn;
//			arg->_networkIn = NULL;
//		}

	}
	else if(status == WFCF_OPEN){

//		Serial_Printf(&Serial, "SERVER OPEN [%d] %s %d",
//						  link_id,
//						  ipaddr, portNum);

		restPort_arg_t * arg =  &rp->_rpArg[link_id];
		arg->_rp = rp;
		arg->_link_id = link_id;
		arg->_networkOut = new LoopbackStream(1024);
		arg->_networkOut->done = false;

		arg->_networkIn = new LoopbackStream(1024);
		
		if(arg->_url){
			free(arg->_url);
			arg->_url = NULL;
		}
 
		if(arg->_body){
			free(arg->_body);
			arg->_body = NULL;
		}
 
		arg->_parser.data = (void*)arg;
	 	http_parser_init(&arg->_parser, HTTP_REQUEST);
	}
	else if(status == WFCF_IPD){
 
//		Serial_Printf(&Serial, "SERVER RECV [%d] len:%ld, %s %d",
//						  link_id,
//						  dataLen,
//						  ipaddr, portNum);
//
//			dumpHex(&Serial,(unsigned char*) data, (int) dataLen, 0);

		restPort_arg_t * arg =  &rp->_rpArg[link_id];
  
		if(arg->_networkIn){
			arg->_networkIn->write((const uint8_t*) data, dataLen);
		}
 	
//		dumpHex(&Serial,(unsigned char*) data, (int) dataLen, 0);
 
	}
	else
	{
//		Serial_Printf(&Serial, "SERVER CALLBACK [%d] ",link_id);
	}
}


RESTportMgr::RESTportMgr() {
	
	_running = false;
	
	memset(&_hpCB, 0, sizeof(http_parser_settings));
	_hpCB.on_message_begin 		= on_message_begin;
	_hpCB.on_url 					= on_url;
	_hpCB.on_header_field 		= on_header_field;
	_hpCB.on_header_value 		= on_header_value;
	_hpCB.on_headers_complete 	= on_headers_complete;
	_hpCB.on_body 					= on_body;
	_hpCB.on_message_complete = on_message_complete;
};


bool RESTportMgr::begin(uint16_t portnum){
	_running = false;
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		_rpArg[idx]._link_id = MAX_WIFI_LINKS;
	}
	
	if(!wifiMgr.hasInterNet) {
		//		if(! wifiMgr.start(&Serial)) {
		return false;
		//	 	}
	}
	
	
	const char *strIP = wifiMgr.ipAddr();
	
	if(!wifiMgr.startServer(portnum, httpServerCallback, this)) {
		return false;
	}
	
//	Serial_Printf(&Serial, " REST started. %s port %u ",
//					  strIP, portnum );
	
	_running = true;
	return true;
}

void RESTportMgr::stop()
{
	if(!_running)
		return;
	
	_running = false;
	
	wifiMgr.stopServer();
 
	  for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		  restPort_arg_t * arg = &_rpArg[idx];
		  
		  if(arg->_link_id != MAX_WIFI_LINKS){

			  arg->_link_id = MAX_WIFI_LINKS;
			  delete arg->_networkOut;
			  delete arg->_networkIn;
		  }
	  }
	
//	Serial_Printf(&Serial," REST stopped");
}



void RESTportMgr::shutdownLink(uint8_t link_id){

	if(!_running)
		return;

//	Serial_Printf(&Serial, "shutdownLink [%d] ",link_id );

	restPort_arg_t * arg =  &_rpArg[link_id];
 
	if(arg->_url){
		  free(arg->_url);
		  arg->_url = NULL;
	  }

	  if(arg->_body){
		  free(arg->_body);
		  arg->_body = NULL;
	  }
	  
	  if(arg->_networkOut != NULL){
		  delete arg->_networkOut;
		  arg->_networkOut = NULL;
	  }

	  if(arg->_networkIn != NULL){
		  delete arg->_networkIn;
		  arg->_networkIn = NULL;
	  }

}

void RESTportMgr::loop(){
	if(!_running)
		return;
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		
		restPort_arg_t * arg =  &_rpArg[idx];
		if(arg->_link_id != MAX_WIFI_LINKS){
			
			if(arg->_networkIn != NULL){
				while(arg->_networkIn->available() > 0){
					
					size_t dataLen = arg->_networkIn->available();
					char data[1024];
					
					if(dataLen > sizeof(data))
						dataLen = sizeof(data);
					
					for(int i = 0 ; i < dataLen ;i++){
						data[i] = arg->_networkIn->read();
					}
					
					size_t nparsed = http_parser_execute(&arg->_parser,
																	 &_hpCB,
																	 (const char*) data, dataLen);
					
					if(nparsed != dataLen){
						// flag problem
					}
					
				}
			}
			
			if(arg->_networkOut != NULL){
				
				while(arg->_networkOut->available() > 0){
					
					size_t dataLen = arg->_networkOut->available();
					char data[1024];
					
					if(dataLen > sizeof(data))
						dataLen = sizeof(data);
					
					for(int i = 0 ; i < dataLen ;i++){
						data[i] = arg->_networkOut->read();
					}
					
//					Serial_Printf(&Serial, "SERVER SEND [%d] len:%ld",
//									  arg->_link_id,
//									  dataLen );
					
//					dumpHex(&Serial,(unsigned char*) data, (int) dataLen, 0);
					
					if(	! wifiMgr.sendData(arg->_link_id, (const uint8_t* )data,dataLen))
					{
//						Serial_Printf(&Serial, "SERVER SEND FAIL");
						this->shutdownLink(idx);
						goto next;
					}
				}
				
				if(arg->_networkOut->done){
					wifiMgr.closeConnection(arg->_link_id);
				}
				
			}
			
		next:;
		}
	}
	
}

int RESTportMgr::activePortCount(){
	int count = 0;
	
	if(_running && wifiMgr.hasInterNet){
		
		cipStatus_entry cpd;
		
		for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
			if(wifiMgr.getConnectionInfo(idx, &cpd)) {
			 	if(cpd.isServer && cpd.localPortNum != 0) 	count++;
				
			}
			
		}
	}
	return count;
}
 
String RESTportMgr::doSync(){
	String result;

	tsMgr.syncClockIfNeeded(true);
	
	result =  hdr204;
	
	return result;
}

String RESTportMgr::setJSON(restPort_arg_t * arg){
	String result;
	
	bool success = false;
	
	JSONVar restJSON =  JSON.parse(arg->_body);
	
	if (JSON.typeof(restJSON) == "undefined") {
		
		success = false;
		
	} else  if (restJSON.hasOwnProperty("door")){
		
		bool state = (bool) restJSON["door"];
		
		if(state) {
			stateMgr.receive_event( EV_OPEN);
		} else  {
			stateMgr.receive_event( EV_CLOSE);
		}
		
		success = true;
		
	} else if(restJSON.hasOwnProperty("light")){
		
		bool state = (bool) restJSON["light"];
		
//		Serial_Printf(&Serial, "Set Light %s |%s|",
//						  state?"ON":"OFF",
//						  restJSON["light"]);
	 
		door.setLight1(state);
		success = true;
	}
	
	else {
		result = hdr404;
		
	}
	
	if(success) {
		result =  hdr204;
		
	} else {
		result =  hdr400;
	}
	return result;
}




String RESTportMgr::minstatusJSON() {
	JSONVar jsObject;
	
	jsObject["uptime"] 	= 	tsMgr.upTime();;
	jsObject["door"] 		=  stateMgr.stateText(stateMgr.currentState);
	jsObject["light"] 		=  door.light1();
	jsObject["tempF"] 		= tempSensor.readTempF();
	
	String jsonString = JSON.stringify(jsObject);
	
	const char* headerStr = 	"HTTP/1.1 200 OK\r\n"
	"Content-Type: application/json\r\n"
	"Content-Length: %u\r\n"
	"Connection: keep-alive\r\n"
	"\r\n";
	
	char headerBuff[128] = {0};
	sprintf(headerBuff, headerStr, jsonString.length());
	
	String result = headerBuff + jsonString;
	
	return result;
	
}

String RESTportMgr::statusJSON() {
	
	JSONVar jsObject;
	
 	time_t  date, sunset, sunrise, civilSunRise, civilSunSet;
	pref_bits_entry prefs;
	ap_entry  apInfo;

	int    gmtOffset;
	double longitude;
	double latitude;
	unsigned long  maxTimeSyncInterval;
	char  ssid[32] = {0};
	
	eeMgr.get_ssid(ssid);
	eeMgr.get_prefBits(&prefs);
	eeMgr.get_timezone(&gmtOffset,&latitude,&longitude);
	
	date 				= tsMgr.timeNow();
	sunset 			= tsMgr.sunSet();
	sunrise 			= tsMgr.sunRise();
	civilSunRise 	= tsMgr.civilSunRise();
	civilSunSet 	= tsMgr.civilSunSet();
	maxTimeSyncInterval = eeMgr.get_timeSyncInterval();
	
//	time_t gmtTime = date - (gmtOffset * SECS_PER_HOUR);
	unsigned long upTime = tsMgr.upTime();
 	
	char timeStr[32] = {0};
	char sunsetStr[32] = {0};
	char sunriseStr[32] = {0};
	char civilSunRiseStr[32] = {0};
	char civilSunSetStr[32] = {0};
  
	char*  timeServerDNS = NULL;

	eeMgr.get_timeServerName(&timeServerDNS);

 	formatRFC3339(date, gmtOffset, (char*)timeStr);
	formatRFC3339(sunset, gmtOffset, (char*)sunsetStr);
	formatRFC3339(sunrise, gmtOffset, (char*)sunriseStr);
	formatRFC3339(civilSunRise, gmtOffset, (char*)civilSunRiseStr);
	formatRFC3339(civilSunSet, gmtOffset, (char*)civilSunSetStr);
 
	jsObject["version"] 		=  versionString;

	jsObject["time"] 			=  timeStr;
	jsObject["uptime"] 		= upTime;

	jsObject["sunset"] 		=  sunsetStr;
	jsObject["sunrise"] 		=  sunriseStr;
	jsObject["civilSunRise"] =  civilSunRiseStr;
	jsObject["civilSunSet"] 	=  civilSunSetStr;
	
	jsObject["gmtoffset"] 	= gmtOffset;
	jsObject["latitude"] 		= latitude;
	jsObject["longitude"] 	= longitude;
	jsObject["useDST"] 		= prefs.useDST;
	jsObject["timeSyncInterval"]  =  maxTimeSyncInterval;

	jsObject["ip"] = wifiMgr.ipAddr();
	jsObject["mac"] = wifiMgr.macAddr();
	jsObject["ssid"]  =  ssid;

	if(wifiMgr.getCurrentAPInfo(&apInfo)) {
		jsObject["rssi"]  =  apInfo.rssi;
		jsObject["apmac"]  =  apInfo.mac;
	}

 	jsObject["ntpserver"]  =  timeServerDNS;
	 
	if(powerMgr.hasSensor()){
		float  voltage = powerMgr.voltage();
		float  current = 	powerMgr.average_current_mA();
		jsObject["voltage"] = voltage;
		jsObject["current"] = current;
	}

	jsObject["acpower"] = powerMgr.hasACpower();
	jsObject["enclosure"] 	= door.enclosureDoorClosed();
 
	bool hasRed = buttons.isConnected(BUTTON_RED);
	bool hasGreen = buttons.isConnected(BUTTON_GREEN);

	if(hasRed && hasGreen)
		jsObject["buttons"] 	= "connected";
	else if(hasRed && !hasGreen)
		jsObject["buttons"] 	= "no green";
	else if(!hasRed && hasGreen)
		jsObject["buttons"] 	= "no red";
	else if(!hasRed && !hasGreen)
		jsObject["buttons"] 	= "not connected";
	
	jsObject["door"] =  stateMgr.stateText(stateMgr.currentState);
	jsObject["light"] =  door.light1();
	jsObject["tempF"] = tempSensor.readTempF();
		
	String jsonString = JSON.stringify(jsObject);
 
	const char* headerStr = 	"HTTP/1.1 200 OK\r\n"
									"Content-Type: application/json\r\n"
									"Content-Length: %u\r\n"
									"Connection: keep-alive\r\n"
									"\r\n";
 
	char headerBuff[128] = {0};
	sprintf(headerBuff, headerStr, jsonString.length());
  
	String result = headerBuff + jsonString;
 
	if(timeServerDNS) free(timeServerDNS);
	
	return result;
}

