//
//  ESP8266Mgr.hpp
//  ESP8266Mgrtest
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

#ifndef ESP8266Mgr_hpp
#define ESP8266Mgr_hpp

#include <Arduino.h>
#include "WifiParser.h"

extern "C" {

typedef enum {
	WFCF_INVALID = 0,
	WFCF_CLOSED,
	WFCF_OPEN,
	WFCF_IPD,
	WFCF_TIMEOUT,		// udp timeout expired
}wcf_status;

typedef void (*connection_func_t)(uint8_t link_id,
											 void* 		arg,
											 int		 	status,  //wcf_status
											 char			ipaddr[16],
											 int 			portNum,
									const  uint8_t*  	data,
											 size_t		dataLen);
};

typedef struct {
 const char* ssid;
 const char* pwd;
} AP_pwd_entry;

#define MAX_WIFI_LINKS 5

class ESP8266Mgr {
	
public:
	
	ESP8266Mgr(Stream* wifiSerial, uint8_t resetPin );
 
	bool begin();
	void stop();
	bool isRunning();

	bool resetDevice();
 
	String getVersion();

	// Access point
	
	/*
	 failReason
	 ‣ 1: connection timeout.
	 ‣ 2: wrong password.
	 ‣ 3: cannot find the target AP.
	 ‣ 4: connection failed.
	 */

	bool connectToAP(const char* ssid, const char* password,  int* failReasonOut = NULL);
	bool connectToKnowAP(AP_pwd_entry* apList, size_t apCount, int* failReasonOut = NULL);
	
	//	 getCurrentAPInfo returns valid  (ssid, mac, channel, rssi) of connected AP
	bool getCurrentAPInfo(ap_entry* apInfo);
	 
	bool disconnectFromAP();
	bool isConnectedToAP();
	
	bool getAPList(bool filterBestRSSI,
						ap_entry** outAllocAPlist,  size_t *outCount);

	// server
	bool startServer(uint16_t portNum,
							  connection_func_t listener, void* arg);

	bool stopServer();

  //  client connection
	bool openConnection(bool isTCP, const char* ipaddr, uint16_t portNum,
							  connection_func_t listener, void* arg,
							  uint8_t* link_idOut );
	
	bool getConnectionInfo(uint8_t  link_id , cipStatus_entry* info);

	bool closeConnection(uint8_t  link_id );

	// data transfer
	bool sendUDP(uint8_t  link_id, uint32_t timeout, const uint8_t* data, size_t dataLen) ;

	bool sendData(uint8_t  link_id, const uint8_t* data, size_t dataLen);

	// call this often in main loop
	void idleHandler(bool *APChangeOut = NULL);
	
	// info
	String ipaddr;
	String mac;

private:
	Stream	*		_wifiSerial;
	uint8_t 		_resetPin;
	WifiParser	 _parser;
	
	connection_func_t	serverCallback;
	void*					serverCallbackArg;
	
	cipStatus_entry		_activeLinks[MAX_WIFI_LINKS];
	connection_func_t 	_linkCallbacks[MAX_WIFI_LINKS];
	void* 	 				_linkCallbackArgs[MAX_WIFI_LINKS];
	unsigned long	 	_udpReplyTimeouts[MAX_WIFI_LINKS];
	bool					_linksToClose[MAX_WIFI_LINKS];
	bool					_newServerConnection[MAX_WIFI_LINKS]; // has the server been notified of connection

	bool _radioIsPowered;
	bool _isSetup;
	bool _isConnectedToAP;
	bool _hasCIFSR;

	bool updateStatus();
	bool softReset();
	void setRadioPower(bool state);
	bool getCIFSR();
	void closeAllLinks();
	bool processLazyLinkClose();
	void processLinkTimeouts();
	
	bool doCmd(const char *fmt, ...);

	wf_resulr_t handleResponse(uint32_t timeout,
										bool *statusChangeOut,
										bool *APChangeOut );
	
	wf_resulr_t recvResponse(uint32_t timeout);
	void writeCommand(const char *fmt, ...);

	
//	bool  handleIPD();
	
 void dumpLinkTable();
 
};


#endif /* ESP8266Mgr_hpp */
