//
//  WifiMgr.cpp
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

#include "WifiMgr.h"
#include "CommonIncludes.h"
 

WifiMgr::WifiMgr(){
	hasInterNet = false;
	_stayConnectedToAP = false;
	_APConnAttempts = 0;
};


void WifiMgr::begin(Stream* serial, uint8_t resetPin){

//	DPRINTF(" WifiMgr::begin");

	hasInterNet = false;
	_stayConnectedToAP = false;
	_APConnAttempts = 0;

	_wifi = new ESP8266Mgr(serial, resetPin);
}

void WifiMgr::end(){
	
//	DPRINTF(" WifiMgr::end");

	hasInterNet = false;
	_stayConnectedToAP = false;

	if(_wifi){
		_wifi->stop();
		_wifi = NULL;
	}
}

bool WifiMgr::isRunning(){
	return _wifi->isRunning();
}


bool WifiMgr::setPower(bool isOn){
	
//	DPRINTF(" WifiMgr::setPower(%u)", isOn);

	bool status = false;
	
	if(isOn) {
		if(_wifi->isRunning())
			status = true;
		else {
			hasInterNet = false;
			status = _wifi->begin();
			
			if(!status){
//				DPRINTF("WIFI device failed to start");
			}

		}
	}
	else {
		_wifi->stop();
		_stayConnectedToAP = false;
		hasInterNet = false;
		status = true;
	}
	return status;
}


void WifiMgr::setAPConnectionONwithOption(wifi_option_t option){
	
	if(!hasInterNet){
	 	_connectOption = option;
	}
	this->setAPConnection(true);
}

// higher level Connect to AP call;
void WifiMgr::setAPConnection(bool isOn){
	
	if(isOn) {
		if(_stayConnectedToAP ){
			// already attempting connections
			return;
		}
		
		if( _APConnAttempts > 0){
			// already attempting connections
			return;
		}
		
		_stayConnectedToAP = true;
		_APConnAttempts = 0;
		
		if(hasInterNet){
			// already  connected
			return;
		}
		
		// try and connect next cycle?
		
	}
	else {
		_APConnAttempts = 0;
		_stayConnectedToAP = false;
		_connectOption = WFOPT_NONE;
		hasInterNet = false;

		if(this->isConnectedToAP()){
			// kill the remote port if its there?
			telnetPortMgr.stop();
			restPortMgr.stop();
			this->disconnectFromAP();
	 		this->setPower(false);
		}
	}
}

bool WifiMgr::connectToAP(int* failReasonOut){
	
	if(!_wifi->isRunning())
		return false;
	
	hasInterNet = false;
	
	char  ssid[32] = {0};
	char  wifiPwd[64] = {0};
	
	eeMgr.get_ssid(ssid);
	eeMgr.get_wifiPwd(wifiPwd);
	
	if(strlen(ssid) == 0){
//		DPRINTF(" NO SSID set");
		return false;
	}
	
	DPRINTF(" Connecting to %s",ssid);
	
	// disable the watchdog. // this takes time
	am_hal_wdt_halt();
	
	bool didConnect =  _wifi->connectToAP(ssid,
													  wifiPwd[0] != '\0'?wifiPwd:"",
													  failReasonOut);
	am_hal_wdt_start();
	
	if(! didConnect ){
		DPRINTF(" wifi.connectToAP failed");
		return false;
	}
	
	hasInterNet = _wifi->isConnectedToAP();
	
	return hasInterNet;
}


bool WifiMgr::isTryingToConnect(int *attemptCountOut){
	
	bool isTrying =  (!hasInterNet && _stayConnectedToAP);
	
	if(isTrying && attemptCountOut)
		*attemptCountOut = _APConnAttempts;
		
	return isTrying;
 }


void WifiMgr::loop(){
 
	const uint32_t connectionStatus_Delay 	= 15*1000;			// 30 seconds
	const uint32_t firstConnection_Delay 	= 5*1000;			//30 seconds
	const uint32_t secondConnection_Delay 	= 5 * 60 * 1000;	//5 minutes
	const uint32_t thirdConnection_Delay 	=  15 * 60 * 1000;	//15 minutes

	bool APstatusChanged = false;
	uint32_t now = millis();
 
	if(_wifi->isRunning()){
		
		_wifi->idleHandler(&APstatusChanged);

 		if( APstatusChanged
			|| (now > (_lastStatusUpdate + connectionStatus_Delay))){
	 
			ap_entry  apInfo;
			if(_wifi->getCurrentAPInfo(&apInfo)) {
				
				hasInterNet = true;
				_rssi = apInfo.rssi;
			}
			else{
				// kill the remote port if its there?
				telnetPortMgr.stop();
				restPortMgr.stop();
				hasInterNet = false;
			}
			
			_lastStatusUpdate = millis();
 		}
 	}
  
	// should we be connected?
	if(!hasInterNet && _stayConnectedToAP){
 
			uint32_t connectionDelay = 0;
		
		// connection strategy
		// first time = right away
		// next 4 every 30 seconds
		// next 4 every 5 minutes
		// after that every 15 minutes
	
		if(_APConnAttempts == 0){
			connectionDelay = 0;
		}else if (_APConnAttempts < 5) {
			connectionDelay = firstConnection_Delay;
		}
		else if (_APConnAttempts < 10) {
			connectionDelay = secondConnection_Delay;
		}
		else {
			connectionDelay = thirdConnection_Delay;
		}
		
	//	DPRINTF("Try to connect in %d seconds", connectionDelay/1000);
		
		if( now > (_lastConnAttempt + connectionDelay)){
	
			// is power on?
			if(!_wifi->isRunning()){
				
				// attempt to power on Wifi
				if(!this->setPower(true)){
					// device init failed!
					// power down and try again.
					_wifi->stop();
					_APConnAttempts = 0;
					_lastConnAttempt = millis();
					return;
				}
			}
			
			//  try to connect
			display.updateDisplay(true);
			if(this->connectToAP()){
				// success!  -
				// see what services need to be started?
				
				_APConnAttempts = 0;
				_lastConnAttempt = 0;
				
				// handle reconnection tasks
				
				if(_connectOption == WFOPT_NTP_ONLY){
					// just do sync
					_connectOption = WFOPT_NONE;
					tsMgr.syncClockIfNeeded(true);
				}
				else {
					uint16_t portnum;
					eeMgr.get_remoteTermPort(&portnum);
					
					if((portnum != 0) && (portnum != 0xffff)){
						pref_bits_entry prefs;
						eeMgr.get_prefBits(&prefs);
						
						if(prefs.remoteTermOnStartup) {
							telnetPortMgr.begin(portnum);
						}else if(prefs.remoteRESTOnStartup) {
							restPortMgr.begin(portnum);
						}
					}
					
					//maybe  NTP sync on reconnect
					tsMgr.syncClockIfNeeded(false);
				}
				
			}
			// connection failed!
			else {
				// power down and try again later.
				_wifi->stop();
			}
			
			_lastConnAttempt = millis();
			_APConnAttempts++;
		}
	}
 }


//  C String wrapper

const char* WifiMgr::ipAddr(){
	
	if(!_wifi->isRunning() || !_wifi->isConnectedToAP())
		return NULL;
 
	return _wifi->ipaddr.c_str();
}

const char* WifiMgr::macAddr(){
	
	if(!_wifi->isRunning() || !_wifi->isConnectedToAP())
		return NULL;
 
	return _wifi->mac.c_str();
}

// wrappers of ESP8266Mgr calls we use

// AP connection
bool WifiMgr::isConnectedToAP(){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->isConnectedToAP();
}

bool WifiMgr:: getCurrentAPInfo(ap_entry* apInfo){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->getCurrentAPInfo(apInfo);
}

bool WifiMgr::connectToAP(const char* ssid, const char* password,  int* failReasonOut)
{
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->connectToAP(ssid,password, failReasonOut);
}

bool WifiMgr::disconnectFromAP(){
	if(!_wifi->isRunning()) return false;
	hasInterNet = false;
	return _wifi->disconnectFromAP();
}


bool WifiMgr::getAPList(bool filterBestRSSI,
								ap_entry** outAllocAPlist,  size_t *outCount){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->getAPList(filterBestRSSI,outAllocAPlist,outCount);
}

//  client connection
bool WifiMgr::openConnection(bool isTCP, const char* ipaddr, uint16_t portNum,
									  connection_func_t listener, void* arg,
									  uint8_t* link_idOut ){
	if(!_wifi->isRunning()) return false;
	
	return _wifi->openConnection(isTCP,ipaddr, portNum, listener, arg, link_idOut);
}

bool WifiMgr::getConnectionInfo(uint8_t  link_id , cipStatus_entry* info){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->getConnectionInfo(link_id,info);
}


bool WifiMgr::closeConnection(uint8_t  link_id ){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->closeConnection(link_id);
}


// server
bool WifiMgr::startServer(uint16_t portNum,
								  connection_func_t listener, void* arg){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->startServer(portNum,listener,arg);
}

bool WifiMgr::stopServer(){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->stopServer();
}


// data transfer
bool WifiMgr::sendUDP(uint8_t  link_id, uint32_t timeout, const uint8_t* data, size_t dataLen){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->sendUDP(link_id, timeout, data, dataLen );
}


bool WifiMgr::sendData(uint8_t  link_id, const uint8_t* data, size_t dataLen){
	
	if(!_wifi->isRunning()) return false;
	
	return _wifi->sendData(link_id,data,dataLen);
}

