//
//  WifiMgr.hpp
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

#ifndef WifiMgr_hpp
#define WifiMgr_hpp

#include <Arduino.h>
#include "ESP8266Mgr.h"

typedef enum {
	WFOPT_NONE = 0,
	WFOPT_NTP_ONLY,		// just for NTP Sync
 }wifi_option_t;

class WifiMgr
{
	
public:
	
	WifiMgr();
	
	void begin(Stream* serial,  uint8_t resetPin );
	void end();
	
 	bool setPower(bool isOn);		// turn on/radio and initialize it
	bool isRunning();				// wifi has power and is running
	
	void loop();
	bool hasInterNet;

	// higher level Connect to AP call;
	void setAPConnection (bool isOn);
	void setAPConnectionONwithOption(wifi_option_t option);
  
	bool isTryingToConnect(int *attemptCount = NULL);
	
	// wrappers of ESP8266Mgr calls we use

	// AP connection
	bool isConnectedToAP();
	bool getCurrentAPInfo(ap_entry* apInfo);
		 
	bool getAPList(bool filterBestRSSI,
						ap_entry** outAllocAPlist,  size_t *outCount);

	//  client connection
	 bool openConnection(bool isTCP, const char* ipaddr, uint16_t portNum,
								connection_func_t listener, void* arg,
								uint8_t* link_idOut );
	 
	 bool getConnectionInfo(uint8_t  link_id , cipStatus_entry* info);

	 bool closeConnection(uint8_t  link_id );
	 
	// server
	bool startServer(uint16_t portNum,
							  connection_func_t listener, void* arg);
 	bool stopServer();
  
	// data transfer
	bool sendUDP(uint8_t  link_id, uint32_t timeout, const uint8_t* data, size_t dataLen) ;

	bool sendData(uint8_t  link_id, const uint8_t* data, size_t dataLen);

	
	// info wrappers
	const char* ipAddr();
	const char* macAddr();
 
private:
	
	// use setAPConnection to toggle these
	bool connectToAP(int* failReasonOut = NULL);
	bool connectToAP(const char* ssid, const char* password,  int* failReasonOut = NULL);
	bool disconnectFromAP();

	bool			_stayConnectedToAP;
wifi_option_t	_connectOption;
	int				_APConnAttempts;
	uint32_t	 	_lastStatusUpdate;
	uint32_t	 	_lastConnAttempt;
	int 			_rssi; // latest signal strength.

	ESP8266Mgr*  _wifi;

};

#endif /* WifiMgr_hpp */
