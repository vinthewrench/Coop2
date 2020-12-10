//
//  ESP8266Mgr.cpp
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

#include "ESP8266Mgr.h"
#include "Utilities.h"

#include <string>
#include <search.h> // for the qsort()
 
#define DEBUG_PROTOCOL 	0
#define DEBUG_STREAM 	 	0

#if DEBUG_STREAM
#define START_TRACE 	trace_stream = true;
#define STOP_TRACE 	trace_stream = false;
#else
#define START_TRACE
#define STOP_TRACE
#endif

const char atAT[]			PROGMEM	= "AT";
const char atRST[]			PROGMEM	= "AT+RST";
const char atGMR[] 			PROGMEM	= "AT+GMR";
const char atATE0[] 		PROGMEM	= "ATE0";
const char atCWMODE[] 		PROGMEM	= "AT+CWMODE_CUR";
const char atCIPMUX[] 		PROGMEM	= "AT+CIPMUX";
const char atCIPDINFO[] 	PROGMEM	= "AT+CIPDINFO";
const char atCWJAP[] 		PROGMEM	= "AT+CWJAP_CUR";
const char atCIFSR[] 		PROGMEM	= "AT+CIFSR";
const char atCWAUTOCONN[]  PROGMEM	= "AT+CWAUTOCONN";
const char atCIPSTATUS[]  PROGMEM	= "AT+CIPSTATUS";
const char atCWQAP[]  		PROGMEM	= "AT+CWQAP";
const char atCWLAP[]  		PROGMEM	= "AT+CWLAP";
const char atCIPSTART[]  		PROGMEM	= "AT+CIPSTART";
const char atCIPSEND[]  		PROGMEM	= "AT+CIPSEND";
const char atCIPCLOSE[]  		PROGMEM	= "AT+CIPCLOSE";
const char atCIPSERVER[]  	PROGMEM	= "AT+CIPSERVER";

void niceDelay(unsigned long duration){
	unsigned long startMillis = millis();
	while(millis() - startMillis < duration){
		sqrt(4700);
	}
}


/*
 AT+RST
 
 AT+CWMODE_CUR=1
 AT+CIPMUX=1
 AT+CWAUTOCONN=0
 AT+CIPDINFO=1
 AT+GMR
 
 // connect UDP ipaddr, portum == > <conn Id>
 AT+CIPSTART=0,"UDP","north-america.pool.ntp.org",123
 == 0,CONNECT
 == OK
 ---------------
 
 send <conn id> bytes
 AT+CIPSEND=0,48
 ==  OK
 == >
 
 1B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 ==>Recv 48 bytes
 
 ==>  SEND OK
 -----------
 +IPD,0,48,163.237.218.19,123:
 2E 2E 2E E7 2E 2E 2E 2E 2E 2E 2E 2E 47 50 53 2E E3 2E D7 89 2E 2E 2E 2E 2E 2E 2E 2E 2E 2E 2E 2E E3 2E D7 89 DB 83 2E 79 E3 2E D7 89 DB 83 6C 75
 -----------
 
 close <conn id>
 
 AT+CIPCLOSE=0
 == 0,CLOSED
 == OK
 
 */


 #if DEBUG_PROTOCOL

static const char* responseTable[] = {
	"INVALID",
	"OK",
	"ERROR",
	"FAIL",
	"WIFI_CONNECTED",
	"WIFI_GOT_IP",
	"DISCONNECT",
	"BUSY_S",
	"BUSY_P",
	"CONNECT",
	"CLOSED",
	"IPD",
	"SEND_OK",
	"SEND_FAIL",
	"STAIP",
	"STAMAC",
	"APIP",
	"APMAC",
	"CWLAP",
	"CJLAP",
	"CIPSTATUS",
	"SEND_DATA",
	"OTHER",
	"READY",
	"CONTINUE",
	"NOTHING"
};
 #endif


#if DEBUG_STREAM
static bool trace_stream = false;
#endif

// MARK: - ESP8266Mgr

ESP8266Mgr::ESP8266Mgr(Stream* wifiSerialIn, uint8_t resetPin ){
	
	_wifiSerial = wifiSerialIn;
	_resetPin 			= resetPin;
	_hasCIFSR 			= false;
	_isConnectedToAP  = false;
	_isSetup 			= false;
	_radioIsPowered 	= false;

	pinMode(_resetPin, OUTPUT);

	_parser.reset();
}
bool ESP8266Mgr::begin(){
	
	if(_isSetup)
		return true;
	
	bool success = false;
	
	STOP_TRACE
		
	_parser.reset();
	success = resetDevice();
	
	if(success){
		success = softReset();
	}
	
	// set echo off
	if(success){
		success = doCmd(atATE0);
	}
	
	// set Wi-Fi mode
	if(success){
		success = doCmd("%s=%d",atCWMODE,1);
	}
	
	// set Enable Multiple Connections
	if(success){
		success = doCmd("%s=%d",atCIPMUX,1);
	}
	
	// disable autoconnect
	if(success){
		success = doCmd("%s=%d",atCWAUTOCONN,0);
	}
	
	// Shows remote IP and remote port with +IPD
	if(success){
		success = doCmd("%s=%d",atCIPDINFO,1);
	}
	
	START_TRACE
	
	_isSetup = success;
	
	return success;
}


bool ESP8266Mgr::resetDevice()
{
	constexpr int HW_RESET_RETRIES  = 3;
 
	//Do a HW reset
	bool statusOk = false;
	
	for(int i =0; i<HW_RESET_RETRIES && !statusOk; i++){
		
		setRadioPower(false);
		niceDelay(500);
		setRadioPower(true);
		
		while(true) {
			wf_resulr_t result = recvResponse(3000);
			if(result == WFR_OTHER) continue;
			if(result == WFR_READY){
				statusOk = true;
			}
			break;
		}
	}
	
	return statusOk;
}


bool ESP8266Mgr::softReset(){
 
	constexpr int SW_RESET_RETRIES  = 3;
	bool statusOk = false;
	
	for(int i =0; i<SW_RESET_RETRIES && !statusOk; i++){
		
		//Do a SW reset
		writeCommand(atRST);
		while(true) {
			wf_resulr_t result = recvResponse(5000);
			
			if(result == WFR_READY){
				statusOk = true;
			}
			else if(result == WFR_OK) continue;
			// lots of noise
			else if(result == WFR_OTHER) continue;
			
			break;
		}
	}
 
	return statusOk;
}


void ESP8266Mgr::setRadioPower(bool state){
	
	digitalWrite(_resetPin, state?HIGH:LOW);
	_radioIsPowered = state;
	
	if(!state){
		_isConnectedToAP = false;
		_hasCIFSR = false;
		
		// zero  out the stuff
	 	bzero(_activeLinks, sizeof(cipStatus_entry) * MAX_WIFI_LINKS );
		bzero(_linkCallbacks,sizeof(connection_func_t) * MAX_WIFI_LINKS );
		bzero(_linkCallbackArgs, sizeof (void*) * MAX_WIFI_LINKS );
		bzero(_udpReplyTimeouts, sizeof(unsigned long) * MAX_WIFI_LINKS );
		bzero(_linksToClose,  sizeof(bool) * MAX_WIFI_LINKS );
		bzero(_newServerConnection,  sizeof(bool) * MAX_WIFI_LINKS );

		ipaddr= String();
		mac = String();
	}
}

void ESP8266Mgr::stop(){
	
	setRadioPower(false);
	_isSetup = false;
	
}


bool ESP8266Mgr::isRunning()
{
	return _isSetup;
}

String ESP8266Mgr::getVersion(){
	
	String str = String();

	STOP_TRACE

	writeCommand(atGMR);
	
	while(true) {
		
		wf_resulr_t result = this->handleResponse(3000, NULL,NULL);

		if(result == WFR_OTHER){
			
			uint8_t* data = NULL;
			if(_parser.get_data( &data, NULL)) {
				
//#if DEBUG_PROTOCOL
//				Serial_Printf(&Serial,"OTHER |%s|\n", (char*) data);
//#endif
				str+= (char*)data;
				str+= " ";
				free(data);
			}
			
		}else {
			break;
		}
	}
	START_TRACE
	

	return str;
}


/*
 failReason
 ‣ 1: connection timeout.
 ‣ 2: wrong password.
 ‣ 3: cannot find the target AP.
 ‣ 4: connection failed.
 */

bool ESP8266Mgr::connectToAP(const char* ssid,
								  const char* password,
								  int* 	failReasonOut){
	bool success = false;
	bool askForCIFSR = false;
	int failReason = 0;
	
 	STOP_TRACE

	_isConnectedToAP = false;
	_hasCIFSR = false;
	// zero  out the linkIDs
	bzero(_activeLinks,  sizeof(cipStatus_entry) * MAX_WIFI_LINKS );
	
	ipaddr= String();
	mac = String();
	
	writeCommand("%s=\"%s\",\"%s\"",atCWJAP ,ssid , password? password:"" );
	
	while(true){
		
		wf_resulr_t result = recvResponse(5000);
		
		if(result == WFR_OK){
			_isConnectedToAP = true;
			success = true;
			break;
		}
		else if(result == WFR_FAIL){
			_isConnectedToAP = false;
			break;
		}
		else if(result == WFR_WIFI_CONNECTED){
			_isConnectedToAP = true;
		}
		else if(result == WFR_DISCONNECT){
			_isConnectedToAP = false;
			success = false;
		}
		else if(result == WFR_CJLAP){
 
			uint8_t* data = NULL;
			if(_parser.get_data( &data, NULL)) {
				//   error reason - not always correct
					int result = sscanf((const char *) data, "%d", &failReason);
				free(data);
			}
			
			_isConnectedToAP = false;
			success = false;
		}
		else if(result == WFR_WIFI_GOT_IP){
			askForCIFSR = true;
		}
		// ignore noise
	}
	
	if(success && askForCIFSR){
		success = getCIFSR();
	}
	
	if(!success){
		if(failReasonOut)
			*failReasonOut = failReason;
	}
	
	START_TRACE

	return success;
	
}


 bool ESP8266Mgr::connectToKnowAP(AP_pwd_entry* knownAPList,
										 size_t knownAPCount,
										 int* 	failReasonOut)
{
	bool success = false;

	STOP_TRACE

	int failReason = 0;
	
	ap_entry* foundAPList = NULL;
	size_t count = 0;
	
	if(this->getAPList(true ,&foundAPList, &count)
		&& (count > 0)) {
		
		for(size_t i = 0; i < knownAPCount; i++){
			AP_pwd_entry en = knownAPList[i];
			
			for(size_t j = 0; j < count; j++){
				ap_entry* ap = &foundAPList[j];
				
				if( strncmp(en.ssid, ap->ssid, 32) == 0){
					
//	Serial_Printf(&Serial, "CONNECT TO |%s|", en.ssid);

					if(this->connectToAP(en.ssid,en.pwd, &failReason))
					{
						success = true;
						goto done;
					}
				}
			}
		}
	}
	
	START_TRACE;
	
	if(foundAPList)
		free(foundAPList);
	
done:
	
	if(!success){
		if(failReasonOut)
			*failReasonOut = failReason;
	}

	return success;
}


bool ESP8266Mgr::disconnectFromAP(){

	STOP_TRACE

	_isConnectedToAP = false;
	_hasCIFSR = false;
		
	ipaddr= String();
	mac = String();
	
	bool success = false;
	
	writeCommand(atCWQAP);
	
	while(true){
		
		wf_resulr_t result = this->handleResponse(1000, NULL, NULL);
	 
		if(result == WFR_OK){
			success = true;
			break;
		}
	}
	
//	//  update the table on next idle  -- it takes some time to disconnect
//	this->updateStatus();

	START_TRACE

	return success;
}

bool ESP8266Mgr::isConnectedToAP() {
	
	return (_isSetup
			  && _radioIsPowered
			  && _isConnectedToAP);
 
	return false;
}

//	 getCurrentAPInfo returns valid  (ssid, mac, channel, rssi) of connected AP

bool ESP8266Mgr::getCurrentAPInfo(ap_entry* apInfoOut){
	
	bool success = false;
	bool statusChange = false;

	STOP_TRACE

	if(!this->isConnectedToAP())
		return false;
	
	ap_entry info;
	bzero(&info, sizeof(ap_entry));
 
	// request Query
	writeCommand("%s?",atCWJAP );

	while(true){
		
		wf_resulr_t result = this->handleResponse(2000, NULL, NULL);
		
		if(result == WFR_OK){
			break;
		}
		if(result == WFR_CJLAP){
			
			uint8_t* data = NULL;
			if(_parser.get_data( &data, NULL)) {
				
				int result = sscanf((const char *) data, "\"%32[^\"]\",\"%17[^\"]\",%d,%d)",
										  info.ssid,
										  info.mac,
										  &info.channel,
										  &info.rssi) ;
				
				success = result == 4;
				free(data);
			}
		}
		else if(result == WFR_OTHER){  // No AP
			//			uint8_t* data = NULL;
			//
			//			if(_parser.get_data( &data, NULL)) {
			//				Serial_Printf(&Serial, "CWJAP REPLY |%s|",data);
			//				free(data);
			//			}
		}
		else {
			// undefined
			break;
		}
		
	}

	if(success){
		if(apInfoOut)
			*apInfoOut = info;
	}
	// update status table.
	if(statusChange){
		this->updateStatus();
	}
 	START_TRACE
 
	return success;
}

void ESP8266Mgr::idleHandler(bool *APChangeOut){

	STOP_TRACE
	
	if(!_isSetup || !_radioIsPowered)
		return;
	
	bool statusChange = false;
	bool APstatusDidChange = false;
	bool askForCIFSR = false;	
	
	// check for serial activity
	if(_wifiSerial->available() > 0){
		
		while(true){
			
			wf_resulr_t result = this->handleResponse(0, &statusChange, &APstatusDidChange);
			
			// what we want is nothing
			if(result == WFR_NOTHING){
				break;
			}
			if(result == WFR_WIFI_GOT_IP){
				askForCIFSR = true;
				continue;
			}
			else {
				// not sure?
//				Serial_Printf(&Serial,"HANDLE RESPONSE %i ?",result);
				break;
			}
		}
	}
	
	if(this->processLazyLinkClose()){
		statusChange = true;
	}
	
	this->processLinkTimeouts();

	// update status table.
	if(statusChange){
		this->updateStatus();
	}

	if(askForCIFSR) {
		getCIFSR();
	}
	
	START_TRACE

	if(APChangeOut)
		*APChangeOut = APstatusDidChange;
}

bool ESP8266Mgr::stopServer(){
	
	// are we al;ready stopped?
		if(serverCallback == NULL)
			return true;

	STOP_TRACE

	bool success = false;

	serverCallback = NULL;
	serverCallbackArg = NULL;
	
	writeCommand("%s=%u",atCIPSERVER, 0 );

	while(true){
		
		wf_resulr_t result = recvResponse(1000);
		
		if(result == WFR_OK){
			success = true;
			break;
		}
		else {
			// ignore noise
//			Serial_Printf(&Serial, "STOP SERVER REPLY  %d",result);
		}

	}
	
	START_TRACE

	return success;
}


bool ESP8266Mgr::startServer(uint16_t portNum,
								  connection_func_t listener, void* arg ){
	bool success = false;
	bool askForCIFSR = false;
	
// are we al;ready started
	if(serverCallback != NULL)
		return true;
	
	serverCallback = listener;
	serverCallbackArg = arg;

	uint8_t link_id = MAX_WIFI_LINKS;
	
	// reap any waiting links.
	this->processLazyLinkClose();

	// select an available linkID
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		if(_activeLinks[idx].localPortNum == 0 ){
			link_id = idx;
			break;
		}
	}

	// fail if none available
	if(link_id >= MAX_WIFI_LINKS)
		return false;
	
	writeCommand("%s=%u,%u",atCIPSERVER, 1, portNum );

	while(true){
		
		wf_resulr_t result = this->handleResponse(5000, NULL, NULL);
	 
		if(result == WFR_OK){
			success = true;
			break;
		}
		else if(result == WFR_WIFI_GOT_IP){
			askForCIFSR = true;
		}
		else if(result == WFR_OTHER){
			
			uint8_t* other = NULL;
			
			if(_parser.get_data( &other, NULL)) {
			
//				Serial_Printf(&Serial, "START SERVER REPLY: %s",other);
		
	 			free(other);
			}
		}
		else {
			// ignore noise
//			Serial_Printf(&Serial, "START SERVER REPLY  %d",result);
		}

	}
 
	if(askForCIFSR) {
		getCIFSR();
	}
	
	return success;

}


bool ESP8266Mgr::openConnection(bool isTCP, const char* ipaddr, uint16_t portNum,
									  connection_func_t listener, void* arg, uint8_t* link_idOut ){
	bool success = false;
	
	uint8_t link_id = MAX_WIFI_LINKS;
	
	// reap any waiting links.
	this->processLazyLinkClose();
	
	// select an available linkID
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		if(_activeLinks[idx].localPortNum == 0){
			link_id = idx;
			break;
		}
	}
	
	// fail if none available
	if(link_id >= MAX_WIFI_LINKS)
	{
		Serial_Print(&Serial, "NO LINKS AVAILABLE");
#if DEBUG_PROTOCOL
		dumpLinkTable();
#endif
		return false;
	
	}
	
	writeCommand("%s=%u,\"%s\",\"%s\",%u",atCIPSTART , link_id,
					 isTCP?"TCP":"UDP", ipaddr, portNum );
	
	while(true){
		
		wf_resulr_t result = this->handleResponse(5000, NULL, NULL);

		if(result == WFR_OK){
			success = true;
			break;
		}
		else if(result == WFR_ERROR){
			break;
		}
//		else {
//			Serial_Printf(&Serial,"openConnection response %u %s\n", result, responseTable[result]);
//
//			// ignore noise
//
//		}
		
	}
	
	// update the link table.
	if(success){
		success = this->updateStatus();
	}
	if(success){
		
		_linkCallbacks[link_id] = listener;
		_linkCallbackArgs[link_id] = arg;
 
		if(link_idOut)
			*link_idOut = link_id;
	}
	return success;
}



bool ESP8266Mgr::getConnectionInfo(uint8_t  link_id , cipStatus_entry* info){
	
	// param check
	if(link_id >= MAX_WIFI_LINKS)
		return false;
	
	if(_activeLinks[link_id].localPortNum == 0 ){
		return false;
	}
	
	if(info)
		*info = _activeLinks[link_id];
	
	return true;
}

bool ESP8266Mgr::closeConnection(uint8_t  link_id ){
//	bool success = false;
	
	// param check
	if(link_id >= MAX_WIFI_LINKS)
		return false;
	
	if(_activeLinks[link_id].localPortNum == 0 ){
		return false;
	}
 
// we defer the close to a later time.
	_udpReplyTimeouts[link_id] = 0;
	_linkCallbacks[link_id] = NULL;
	_linkCallbackArgs[link_id] = NULL;
	_linksToClose[link_id] = true;
	_newServerConnection[link_id] = false;
	
	return true;
}

bool ESP8266Mgr::sendData( uint8_t  link_id, const uint8_t* data, size_t dataLen){
	bool success = false;

//	Serial_Printf(&Serial,"sendData");
// 	dumpHex(&Serial,(uint8_t*)data,dataLen,0);

	int recvCount = 0;
	
	// param check
	if(link_id >= MAX_WIFI_LINKS)
		return false;
	
	if(_activeLinks[link_id].localPortNum == 0 ){
		return false;
	}
	
	/* send works like this:
		 -->  AT+CIPSEND=<linkid>,<bytecount>
		 <--  OK
		 <--  "> "
		 --> <data bytes>
		 <--  Recv <bytecount> bytes
		 <--  SEND OK
	 */
	
	writeCommand("%s=%u,%u",atCIPSEND ,link_id, dataLen);

	// wait for ready to send.
	while(true){
//		wf_resulr_t result = recvResponse(500);
		
		wf_resulr_t result = this->handleResponse(1000, NULL, NULL);
	
		if(result == WFR_OK) {
			// ignore for now -- this is the ack of the send commend
		}
		else if(result == WFR_ERROR) {
 		// fail this
			break;
		}
		else if(result == WFR_SEND_DATA)
		{
			// signal to send
			_wifiSerial->write(data, dataLen);
		}
		else if(result == WFR_OTHER){  // expect  "Recv %d bytes"
			
			uint8_t* other = NULL;
			
			if(_parser.get_data( &other, NULL)) {
				sscanf((const char *) other, "Recv %d bytes", &recvCount );
				free(other);
			}
	 		 else {
					
//				 Serial_Printf(&Serial,"sendData Unexpected  |%s|", other);
				 free(other);
			}
		}
		else if(result == WFR_SEND_OK){   // send was OK
			success = true;
			break;
		}
		else
		{
			break;   // anything else is fail..
		}
	}
	
	// check sent bytecount
	if(success) {
		success = ((size_t)recvCount == dataLen);
	}
	
//	Serial_Printf(&Serial,"sendData %s",success?"OK":"Fail");

	return success;
}

bool ESP8266Mgr::sendUDP(uint8_t link_id, uint32_t timeout, const uint8_t* data, size_t dataLen) {
	bool success = false;
	
	success = sendData(link_id,data,dataLen);
	
	if(success) {
		
		unsigned long now = millis();
		_udpReplyTimeouts[link_id] = now + timeout;
	}
	
	return success;
}


// MARK: - ESP8266 API

// get IP address and stuff
bool ESP8266Mgr::getCIFSR(){
	bool success = false;

	STOP_TRACE

	writeCommand(atCIFSR);
	
	while(true){
		
		wf_resulr_t result = recvResponse(3000);
		
		if(result == WFR_OK){
			_hasCIFSR = true;
			success = true;
			break;
		}
		else if(result == WFR_STAIP){
			ipaddr = _parser.get_ipaddr();
		}
		else if(result == WFR_STAMAC){
			
			mac = _parser.get_mac();
			
		}
		else break;
	}

	START_TRACE
	
	return success;
}

#if DEBUG_PROTOCOL

void ESP8266Mgr::dumpLinkTable() {

	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){

		cipStatus_entry  cpd = _activeLinks[idx];
		Serial_Printf(&Serial, "%u: %c %u %-16s %5d - %5d %s",
						  idx, cpd.isServer == 1?'S':'C',
						  cpd.connType,
						  cpd.ipaddr,
						  cpd.remotePortNum,
						  cpd.localPortNum,
						  _linksToClose[idx]?"CLOSE":""
						  );
	}
}
#endif

bool ESP8266Mgr::processLazyLinkClose(){
 
	if(!_radioIsPowered)
		return false;
	
	bool a_link_was_closed = false;
	
	// check for defeered link close
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		// perform link close here
		if(_linksToClose[idx]) {
			
			a_link_was_closed = true;
			_linksToClose[idx] = false;
			_udpReplyTimeouts[idx] = 0;
			
//	Serial_Printf(&Serial,"atCIPCLOSE  %u\n", idx);

			writeCommand("%s=%u",atCIPCLOSE , idx);
	
			while(true){
				
				wf_resulr_t result = this->handleResponse(1000, NULL, NULL);
				
				if(result == WFR_OK){
					break;
				}
				else if(result == WFR_CLOSED){
					continue;
				}
				else if(result == WFR_ERROR){
					break;
 				}
				else if(result == WFR_UNLINK){
//					Serial_Printf(&Serial,"UNLINK");
 					continue;
				}
				else if(result == WFR_NOTHING){
//					Serial_Printf(&Serial,"closeConnection WFR_NOTHING");
					break;
				}
	 
				else {
//// DEBUG
	//					Serial_Printf(&Serial,"closeConnection RESPONSE %i ?",result);
			
//			Serial_Printf(&Serial,"closeConnection response %u %s\n", result, responseTable[result]);

					//	everything else is an error
					continue;
				}
			}
		}
	}

 
	return a_link_was_closed;
}

void ESP8266Mgr::processLinkTimeouts(){
	// check for timeouts
	unsigned long now = millis();
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		
		// is it an active UPD connection
		if( (_activeLinks[idx].localPortNum != 0)
			&& (_activeLinks[idx].isServer == 0)
			&& (_activeLinks[idx].connType == CIPC_UDP)
			&& (_udpReplyTimeouts[idx] > 0))
		{
			// did it timeout
			if(_udpReplyTimeouts[idx] < now){
				
				// inform listner of timeout
				if(_linkCallbacks[idx] != NULL) {
					
					(_linkCallbacks[idx])(idx, _linkCallbackArgs[idx],
												 WFCF_TIMEOUT,
												 _activeLinks[idx].ipaddr,
												 _activeLinks[idx].remotePortNum,
												 NULL, 0 );
				}
				_udpReplyTimeouts[idx] = 0;
			}
		}
	}

}


bool ESP8266Mgr::updateStatus(){
	
	bool success = false;
	bool done = false;
	
	bool shouldCLoseLinks = false;

	// 	Serial_Print(&Serial,"updateStatus");
	
	// zero  out the linkIDs
	cipStatus_entry updatedLinks[MAX_WIFI_LINKS];
	bzero(updatedLinks, sizeof(cipStatus_entry) * MAX_WIFI_LINKS );
	
	writeCommand(atCIPSTATUS);
	while(!done){
		
		// we dont care about statusChange ..
		
		wf_resulr_t result = this->handleResponse(1000, NULL, NULL);
		
		// the CIPSTATUS command is done
		if(result == WFR_OK){
			done = true;
			success = true;
			
//			Serial_Print(&Serial,"CIPSTATUS done");

		}
		
		//  +CIPSTATUS:0,"UDP","173.230.154.254",123,6636,0

		else if(result == WFR_CIPSTATUS){
			// update the _activeLinks
			cipStatus_entry  cip;
	 
			if(_parser.get_cipstatus( &cip)) {
				if(cip.linkid < MAX_WIFI_LINKS){
					
					// check for server here..
					if((cip.isServer == true)
						&& (_linkCallbacks[cip.linkid] == NULL) ){
						_linkCallbacks[cip.linkid] = serverCallback;
						_linkCallbackArgs[cip.linkid] = serverCallbackArg;
						
						if(!_newServerConnection[cip.linkid])
							_newServerConnection[cip.linkid] = true;
					}
					updatedLinks[cip.linkid] = cip;
				}
			}
		}
		else if(result == WFR_OTHER){
			uint8_t* data = NULL;
			
			if(_parser.get_data( &data, NULL)) {
				
				int status = -1;
				int result = sscanf((const char *) data, "STATUS:%d", &status  );
								
				if(result == 1){
					
					switch(status){
						case 2:		// The ESP8266 Station is connected to an AP and its IP is obtained.
						case 3:		// The ESP8266 Station has created a TCP or UDP transmission.
						case 4:		//  The TCP or UDP transmission of ESP8266 Station is disconnected.
							_isConnectedToAP = true;
							break;
							
						case 5:		//  The ESP8266 Station does NOT connect to an AP.
							shouldCLoseLinks = true;
							_isConnectedToAP = false;
							break;

						default:
							_isConnectedToAP = false;
					}
				} else {
					
						Serial_Printf(&Serial,"CIPSTATUS found |%s|", data);
				}
				
				free(data);
			}
		}
		else {
			done = true;
		}
	}
	
	// special case - wifi disconnected from AP.
	// wee need to close all connections on device
	
		// update the link table
	memcpy(_activeLinks, updatedLinks, sizeof(cipStatus_entry) * MAX_WIFI_LINKS );

	if(shouldCLoseLinks){
		this->closeAllLinks();
	}
	
	// tell any server callbacks a new connection is made
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		if(_newServerConnection[idx]) {
			
			(_linkCallbacks[idx])(idx, _linkCallbackArgs[idx],
										 WFCF_OPEN,
										 _activeLinks[idx].ipaddr,
										 _activeLinks[idx].remotePortNum,
										 NULL, 0 );

			_newServerConnection[idx] = false;
		}
	}
	
//	dumpLinkTable();


	return success;
}

void ESP8266Mgr::closeAllLinks(){
	
	// walk the link table
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++){
		
		// if it's an active link, we need to close it
		if(_activeLinks[idx].localPortNum != 0 ){
			
			// close the link on the device
			writeCommand("%s=%u",atCIPCLOSE , idx);
			
			while(true){
				
				wf_resulr_t result = recvResponse(1000);
				
				if(result == WFR_OK){
					break;
				}
				else if(result == WFR_CLOSED){
					continue;
				} else {
					//	everything else is an error
					break;
				}
			}
			
			// call any callbacks
			if(_linkCallbacks[idx] != NULL) {
				
				char noIPaddr[16] = {0};
				(_linkCallbacks[idx])(idx, _linkCallbackArgs[idx],
											 WFCF_CLOSED, noIPaddr, 0,  NULL, 0 );
			}
		}
	}

	// zero  out the linkIDs
	bzero(_activeLinks, sizeof(cipStatus_entry) * MAX_WIFI_LINKS );
	bzero(_linkCallbacks,sizeof(connection_func_t) * MAX_WIFI_LINKS );
	bzero(_linkCallbackArgs, sizeof (void*) * MAX_WIFI_LINKS );
	bzero(_udpReplyTimeouts, sizeof(unsigned long) * MAX_WIFI_LINKS );
	bzero(_linksToClose,  sizeof(bool) * MAX_WIFI_LINKS );
	bzero(_newServerConnection,  sizeof(bool) * MAX_WIFI_LINKS );
 
}


int CompareByRSSI(void* arg,  const void *elem1, const void *elem2 )
{
	ap_entry* ap1 = (ap_entry*)elem1;
	ap_entry* ap2 = (ap_entry*)elem2;
 
	return (ap1->rssi < ap2->rssi ? 1 : -1);		// we are sorting for strongest signal
}

bool ESP8266Mgr::getAPList(bool filterBestRSSI,
								ap_entry** outAllocAPlist,  size_t *outCount){
	
	bool success = false;
	
	const size_t apAlloc_quantum = 1;
	ap_entry	*apList = NULL;
	size_t	 apCount = 0;
	size_t	 apAlloc = 0;
	
	STOP_TRACE
	
	writeCommand(atCWLAP);
	
	while(true) {
		
		wf_resulr_t result = recvResponse(3000);
		
		if(result == WFR_CWLAP){
			
			ap_entry  apd;
			
			if(_parser.get_apInfo(&apd)) {
				
				bool found = false;
				
				//			Serial_Printf(&Serial,"SSID: %s |%17s| %d", apd.ssid, apd.mac, apd.rssi);
				
				if(outAllocAPlist){
					
					// create a table if needed
					if(apList == NULL)
					{
						apAlloc = apAlloc_quantum;
						apList = (ap_entry*) malloc(apAlloc * sizeof(ap_entry));
					}
					
					// scan for existing entry
					for(size_t idx = 0; idx < apCount ; idx++){
						ap_entry* entry = &apList[idx];
						
						// never the same MAC (bug in ESP8266 SW)
						if( strncmp(apd.mac, entry->mac, 17) == 0){
							found = true;
							break;
						}
						
						if(filterBestRSSI){
							if( strncmp(apd.ssid, entry->ssid, 32) == 0) {
								found = true;
								if(apd.rssi > entry->rssi){
									*entry = apd;
									break;
								}
							}
						}
					}
					
					if(!found){
						// extend if needed
						if(apCount >= apAlloc)
						{
							apAlloc += apAlloc_quantum;
							apList = (ap_entry*) realloc(apList, apAlloc * sizeof(ap_entry));
						}
						
						apList[apCount] = apd;
					}
					
				}
				if(!found){
					apCount++;
				}
			}
			
		} else if(result == WFR_OK){
			success = true;
			break;
		}
		else {
			break;
		}
	}
	
	if(success){
		
		qsort_r(apList, apCount, sizeof(ap_entry), NULL,  CompareByRSSI);
		
	}
	else {
		if(apList)
			free(apList);
		
		apList = NULL;
		apCount = 0;
	}
	
	
	if(outAllocAPlist)
		*outAllocAPlist = apList;
	
	if(outCount)
		*outCount = apCount;
	
	START_TRACE
	
	
	return success;
}




//  simple do a command
bool ESP8266Mgr::doCmd(const char *fmt, ...){
	
	char buff[128];
	
	va_list args;
	va_start(args, fmt);
	
	vsprintf(buff, fmt, args);
	
#if DEBUG_PROTOCOL
	Serial_Printf(&Serial, "CMD |%s|",buff);
#endif
	_wifiSerial->print(buff);
	_wifiSerial->println();
	
	va_end(args);
	
	while(true){
		wf_resulr_t result = recvResponse(500);
		
		if(result == WFR_OK)
			return true;
		
		if(result == WFR_OTHER)  // ignore noise
			continue;
		
		break;
	}
	
	return false;
}

void ESP8266Mgr::writeCommand(const char *fmt, ...)
{
	char buff[1024];
	
	va_list args;
	va_start(args, fmt);
	
	vsprintf(buff, fmt, args);
	
#if DEBUG_PROTOCOL
	Serial_Printf(&Serial, "CMD |%s|",buff);
#endif
	_wifiSerial->print(buff);
	_wifiSerial->println();
	
	va_end(args);
}

wf_resulr_t ESP8266Mgr::recvResponse(uint32_t timeout){
	
#if DEBUG_STREAM
	uint8_t buffer[1024] ={0};
	size_t  count = 0;
#endif
 
	wf_resulr_t result = WFR_NOTHING;
	 
	unsigned long start = millis();
//	while (millis() - start < timeout)
	
	while(true) {
		while (_wifiSerial->available() > 0 ) {
			
			uint8_t ch = _wifiSerial->read();
			
			result = _parser.process_char(ch);
			
#if DEBUG_STREAM
			
			buffer[count++] = ch;
			if (ch < 32) {
				Serial_Printf(&Serial, "%02x     = %u", ch, result);
			} else {
				Serial_Printf(&Serial, "%02x |%c| = %u", ch, ch, result);
			}
#endif
			
			if(result != WFR_CONTINUE)
				goto done;
		}
		if(timeout == 0
			||( millis() - start > timeout))
			break;
	}
	
	
done:
#if DEBUG_STREAM
	
	if(trace_stream){
		if(count){
			dumpHex(&Serial,buffer,count,0);
		}
		if(result != WFR_NOTHING)
			Serial_Printf(&Serial,"recvResponse %u %s\n", result, responseTable[result]);

	}
#endif
	
	return result;
}


wf_resulr_t ESP8266Mgr::handleResponse(uint32_t timeout,
												bool *statusChangeOut,
												bool *APChangeOut ){
	
	wf_resulr_t result = WFR_INVALID;
	
	bool statusDidChange = false;
	bool APstatusDidChange = false;

	while(true){
		
		result = recvResponse(timeout);
		
		// process more bytes
		if(result == WFR_CONTINUE)
		{
			continue;
		}
		
		// no data
		else if(result == WFR_NOTHING)  // no input?
		{
			break;
		}
		
		// A connection closed.   just mark it for later
		else if(result == WFR_CLOSED){
			// this might be a remote close.
			uint8_t* data = NULL;
	
			if(_parser.get_data( &data, NULL)) {
				
				int linkid = 0;
				if( sscanf((const char *) data, "%d,CLOSED", &linkid) == 1){
		
					if((linkid < MAX_WIFI_LINKS)
						&& (_linkCallbacks[linkid] != NULL)
						&& (_linksToClose[linkid] != true) ) {
					
						// Tell the callback that it is closed.
						char noIPaddr[16] = {0};
						(_linkCallbacks[linkid])(linkid, _linkCallbackArgs[linkid],
														 WFCF_CLOSED, noIPaddr, 0,  NULL, 0 );
						
						_newServerConnection[linkid] 	= false;
						_linkCallbacks[linkid] 			= NULL;
						_linkCallbackArgs[linkid] 		= NULL;
	
					}
				}
				
				free(data);
			}
			
			statusDidChange = true;
			
			continue;
		}
		
		// A connection opened.   just mark it for later
		else if( result == WFR_CONNECT){
			statusDidChange = true;
			break;
		}
		// wifi connected?  just mark it for later
		else if( result == WFR_WIFI_CONNECTED){
			statusDidChange = true;
			APstatusDidChange = true;
			continue;
		}
		// wifi is broken  just mark it for later
		else if( result == WFR_DISCONNECT){
			statusDidChange = true;
			APstatusDidChange = true;
			continue;
		}
		
		// we got input -- proccess it
		else if( result == WFR_IPD){
			ipd_data_t ipd;
			
			if( _parser.get_ipd(&ipd)) {
				// param check
				
				/*
				 In the case of a server.
				 It is possible for the +IPD to come in immediately  after the CONNECT
				 and before we have had a chance to inform the server of a WFCF_OPEN
				 
				 we know this because
				 
				 if((serverCallback != NULL)
				 && (_linkCallbacks[ipd.linkid] == NULL)
				 && (_linksToClose[ipd.linkid] != true))
				 
				 in this case, we need to simulate this and fudge the table.
				 
				 we have the ipd.linkid,  the ipd.ipaddr, and ipd.portNum so we can
				 pass that to a
				 
				 (serverCallback)(linkid, serverCallbackArg,
											 	 WFCF_OPEN,
				 								ipd.ipaddr,
												 ipd.portNum,
											  NULL, 0 );

				 
				 _linkCallbacks[linkid] = serverCallback;
				 _linkCallbackArgs[linkid] = serverCallbackArg;
				 _newServerConnection[linkid] = true;
 
				 Serial_Printf(&Serial,"rcv IPD %u  %u\n", ipd.linkid, ipd.length);

				 */
				  
 
				
#if 1
				if((serverCallback != NULL)
				&& (_linkCallbacks[ipd.linkid] == NULL)
					&& (_linksToClose[ipd.linkid] != true)){
	 
					_linkCallbacks[ipd.linkid] = serverCallback;
					_linkCallbackArgs[ipd.linkid] = serverCallbackArg;
					_newServerConnection[ipd.linkid] = false;
					
					(serverCallback)(ipd.linkid, serverCallbackArg,
													WFCF_OPEN,
												  ipd.ipaddr,
													ipd.portNum,
												 NULL, 0 );

	 
				}
  
				if((ipd.linkid < MAX_WIFI_LINKS)
					&& (_linkCallbacks[ipd.linkid] != NULL)
					&& (_linksToClose[ipd.linkid] != true) ) {
					
					(_linkCallbacks[ipd.linkid])(ipd.linkid,
														  _linkCallbackArgs[ipd.linkid],
														  WFCF_IPD,
														  ipd.ipaddr,
														  ipd.portNum,
														  ipd.data, ipd.length );
				}
				
//				if((serverCallback != NULL)
//				&& (_linkCallbacks[ipd.linkid] == NULL)
//					&& (_linksToClose[ipd.linkid] != true)) {
//
//					(serverCallback)(ipd.linkid,
//										 serverCallbackArg,
//										 WFCF_IPD,
//										 ipd.ipaddr,
//										 ipd.portNum,
//										 ipd.data, ipd.length );
//				}

#else
				if((ipd.linkid < MAX_WIFI_LINKS)
					&& (_linkCallbacks[ipd.linkid] != NULL)
					&& (_linksToClose[ipd.linkid] != true) ) {
					
					(_linkCallbacks[ipd.linkid])(ipd.linkid,
														  _linkCallbackArgs[ipd.linkid],
														  WFCF_IPD,
														  ipd.ipaddr,
														  ipd.portNum,
														  ipd.data, ipd.length );
				}
#endif
				
			}
			continue;
		}
		// anything else just break;
		else {
			break;
		}
	}
	
	if(statusChangeOut)
		*statusChangeOut = statusDidChange;

	if(APChangeOut)
		*APChangeOut = APstatusDidChange;
 
	return result;
	
}
