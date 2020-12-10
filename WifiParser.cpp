//
//  WifiParser.cpp

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

#include "WifiParser.h"
#include <stdio.h>

#include "Utilities.h"

#define ALLOC_QUANTUM 32


WifiParser::WifiParser() {
	dbuff_init();
}
 
WifiParser::~WifiParser(){
	
	if(dbuf.data)
		free(dbuf.data);
	
	dbuf.used = 0;
	dbuf.alloc = 0;
	dbuf.dataLen = 0;
}



void WifiParser::dbuff_init(){
	
	state = WFS_START;
	dbuf.used = 0;
	dbuf.dataLen = 0;
	dbuf.alloc = ALLOC_QUANTUM;
	dbuf.data =  (uint8_t*) malloc(ALLOC_QUANTUM);
}



bool WifiParser::dbuff_cmp(const char* str){
	if(!str) return
		false;
	
	size_t len =  strlen(str);
	
	if(dbuf.used < len)
		return false;
	
	return (memcmp(dbuf.data, str, len) == 0);
};


bool WifiParser::dbuff_cmp_end(const char* str){
	if(!str) return
		false;
	
	size_t len =  strlen(str);
	
	if(dbuf.used < len)
		return false;
	
	return (memcmp(dbuf.data+dbuf.used-len, str, len) == 0);
};


 
void  WifiParser::reset(){
	state = WFS_START;
	dbuf.used = 0;
	dbuf.dataLen = 0;
 }


size_t WifiParser::data_size(){
	return dbuf.used;
};


bool WifiParser::append_data(void* data, size_t len){
	
	if(len + dbuf.used >  dbuf.alloc){
		size_t newSize = len + dbuf.used + ALLOC_QUANTUM;
		
		if( (dbuf.data = (uint8_t*) realloc(dbuf.data,newSize)) == NULL)
			return false;
		
		dbuf.alloc = newSize;
	}
	
	memcpy((void*) (dbuf.data + dbuf.used), data, len);
	dbuf.used += len;
	dbuf.dataLen = dbuf.used;
 
	return true;
}

bool WifiParser::append_char(uint8_t c){
	return append_data(&c, 1);
}
 

wf_resulr_t WifiParser::process_char( uint8_t ch){
	
	wf_resulr_t retval = WFR_CONTINUE;

//	{
//		static int count = 0;
//		printf("%3i State %u %02x  |%c|\n",count++, state,  ch,ch );
//
//	}
	
	if(!dbuf.data  || dbuf.alloc== 0 ){
		dbuff_init();
	}
	
	hasAPD = false;
	
	switch (state) {
		case WFS_START:
			if(ch == '+'){
				state = WFS_PLUS;
				append_char(ch);
			}
			else if(ch == 0x0d){
				state = WFS_CR;
			}
			else if(ch == '>') {
				state = WFS_SENDING;
				// response to a AT+CIPSEND=<linkid>,<count>
				// is a 0x3E 0x20  ie  "> "
 			}
			else {
				append_char(ch);
				state = WFS_INPUT;
			}
			break;
			
		case WFS_SENDING:
			if(ch == ' '){
				this->reset();
				retval = WFR_SEND_DATA;
			}
			break;
					  
		case WFS_CR:
			if(ch == 0x0d) { // ignore extra cr
				break;
			}else if(ch == 0x0a) {  // expect LF
				if(data_size() >0 ){
					
					if(dbuff_cmp(sOK)){
						retval = WFR_OK;
						this->reset();
 					}
					else if(dbuff_cmp(sERROR)){
						retval = WFR_ERROR;
						this->reset();
					}
					else if(dbuff_cmp(sUNLINK)){
						retval = WFR_UNLINK;
						this->reset();
					}
					else if(dbuff_cmp(sFAIL)){
						retval = WFR_FAIL;
						this->reset();
					}
					else if(dbuff_cmp(sWifiConnect)){
						retval = WFR_WIFI_CONNECTED;
						hasMAC = false;
						hasIP = false;
						hasAPIP = false;
						hasAPMAC = false;
						this->reset();
					}
					else if(dbuff_cmp(sWifiDisconnect)){
						hasMAC = false;
						hasIP = false;
						hasAPIP = false;
						hasAPMAC = false;
						retval = WFR_DISCONNECT;
						this->reset();
					}
					else if(dbuff_cmp(sBusyS)){
						retval = WFR_BUSY_S;
						this->reset();
					}
					else if(dbuff_cmp(sBusyP)){
						retval = WFR_BUSY_P;
						this->reset();
					}
					else if(dbuff_cmp(sWifiGotIP)){
						retval = WFR_WIFI_GOT_IP;
						hasMAC = false;
						hasIP = false;
						hasAPIP = false;
						hasAPMAC = false;
						this->reset();
 					}
					else if(dbuff_cmp(sSendOK)){
						retval = WFR_SEND_OK;
						this->reset();
					}
					else if(dbuff_cmp(sSendFail)){
						retval = WFR_SEND_FAIL;
						this->reset();
					}
					else if(dbuff_cmp(sReady)){
						retval = WFR_READY;
						this->reset();
					}
					else if(dbuff_cmp_end(sConnect)){
						dbuf.used = 0;
						retval = WFR_CONNECT;
					}
					else if(dbuff_cmp_end(sClosed)){
						dbuf.used = 0;
						retval = WFR_CLOSED;
					}
 					else
					{
						dbuf.used = 0;
//						Serial_Printf(&Serial, "???: |%.*s|\n", (int) dbuf.dataLen, (char*) dbuf.data);
//						printf("???: |%.*s|\n", (int) dbuf.dataLen, (char*) dbuf.data);
						retval = WFR_OTHER;
					}
 				}
				state = WFS_START;
			}
			else {
				state = WFS_INVALID;
				retval = WFR_INVALID;
			}
			break;
			
		case WFS_IPD:
			if(dbuf.used >=  dbuf.ipd.expect){   /// overflow
				this->reset();
				state = WFS_INVALID;
				retval = WFR_INVALID;
			}
			else { 	// append byte
				append_char( ch);
			}
			
			// are we done?
			if( dbuf.used == dbuf.ipd.expect){
				// expect user to call reset;
				retval = WFR_IPD;
			}
			break;
			
		case WFS_CJLAP:
			if(ch == 0x0d){
				
				append_char('\0'); // null terminate buffer
	
				// +CWJAP:1
				// +CWJAP:3

				dbuf.used = 0;
				state = WFS_CR;
				retval = WFR_CJLAP;
			}
			else {
				append_char(ch);
			}
			break;


		case WFS_CIPSTATUS:
			if(ch == 0x0d){
				
				append_char('\0'); // null terminate buffer
	
				// +CIPSTATUS:0,"UDP","107.170.0.6",123,1356,0
				dbuf.used = 0;
				state = WFS_CR;
				retval = WFR_CIPSTATUS;
				}
			
			else {
				append_char(ch);
			}
	
			break;

		case WFS_CIFSR:
			if(ch == 0x0d){
				int result = sscanf((const char *) dbuf.data, "STAIP,\"%16[^\"]",  ipaddr);
				if(result == 1){

					// expect user read ip
					hasIP = true;
					dbuf.used = 0;
					state = WFS_CR;
					retval = WFR_STAIP;
					break;
				}

				result = sscanf((const char *) dbuf.data, "STAMAC,\"%17[^\"]",  mac);
				if(result == 1){

					// expect user to  read mac
					hasMAC = true;
					dbuf.used = 0;
					state = WFS_CR;
					retval = WFR_STAMAC;
					break;
				}
				
				result = sscanf((const char *) dbuf.data, "APMAC,\"%17[^\"]",  apmac);
				if(result == 1){

					// expect user to  read mac
					hasAPMAC = true;
					dbuf.used = 0;
					state = WFS_CR;
					retval = WFR_APMAC;
					break;
				}

				result = sscanf((const char *) dbuf.data, "APIP,\"%16[^\"]",  apip);
				if(result == 1){

					// expect user read ip
					hasAPIP = true;
					dbuf.used = 0;
					state = WFS_CR;
					retval = WFR_APIP;
					break;
				}

				this->reset();
				state = WFS_CR;
			}
			else {
				append_char(ch);
			}
				
			break;

		case WFS_CWLAP:
			if(ch == 0x0d){

				bzero(dbuf.apd.ssid,sizeof(dbuf.apd.ssid));
				bzero(dbuf.apd.mac,sizeof(dbuf.apd.mac));
		
				int result =  sscanf((const char *) dbuf.data, "(%d,\"%32[^\"]\",%d,\"%17[^\"]\",%d,%d,%d)",
											&dbuf.apd.ecn,
											dbuf.apd.ssid,
											&dbuf.apd.rssi,
											dbuf.apd.mac,
											&dbuf.apd.channel,
											&dbuf.apd.freq_offset,
											&dbuf.apd.freq_calibration);
				
				// special case - the SSID is null
				if(result == 1){
					result =  sscanf((const char *) dbuf.data, "(%d,\"\",%d,\"%17[^\"]\",%d,%d,%d)",
										  &dbuf.apd.ecn,
										  &dbuf.apd.rssi,
										  dbuf.apd.mac,
										  &dbuf.apd.channel,
										  &dbuf.apd.freq_offset,
										  &dbuf.apd.freq_calibration);
					
					if(result == 6){
						result = 7;
					}
				}
	
				dbuf.used = 0;

				if(result == 7 ){
					retval = WFR_CWLAP;
					state = WFS_CR;
					hasAPD = true;
				}else {
					state = WFS_INVALID;
					retval = WFR_INVALID;
				}
			}
			else {
				append_char(ch);
			}
			break;
			
		case WFS_PLUS:
			if(ch == ':'){
				// check for  IPD/ CWLAP / CIFSR / CIPSTATUS
				
				append_char('\0'); // null terminate buffer
				
				if(dbuff_cmp(sIPD)){
					state = WFS_IPD;
					int link_id;
					int count;
					char ipaddr[16]= {0};
					int portNum = 0;
					
				 	int result = sscanf((const char *) dbuf.data, "+IPD,%d,%d,%16[^,],%d",
											  &link_id,&count,ipaddr,&portNum);
					
					if(result > 3) { // AT+CIPDINFO=1
						dbuf.ipd.expect = count;
						dbuf.ipd.linkid = link_id;
						memcpy(dbuf.ipd.ipaddr, ipaddr,16);
						dbuf.ipd.portNum = portNum;
						this->reset();
						state = WFS_IPD;
					}
					else if(result >1 ){  // AT+CIPDINFO=0
						dbuf.ipd.expect = count;
						dbuf.ipd.linkid = link_id;
						dbuf.ipd.ipaddr[0] = '\0';
						dbuf.ipd.portNum = 0;
						this->reset();
						state = WFS_IPD;
					}
					else {
						this->reset();
						state = WFS_INVALID;
						retval = WFR_INVALID;
					}
				}
				else if(dbuff_cmp(sCIFSR)){
					state = WFS_CIFSR;
					dbuf.used = 0;
 				}
				else if(dbuff_cmp(sCWLAP)){
					state = WFS_CWLAP;
					dbuf.used = 0;
				}
				else if(dbuff_cmp(sCWJAP)){
					state = WFS_CJLAP;
					dbuf.used = 0;
				}
				else if(dbuff_cmp(sCIPSTATUS)){
					state = WFS_CIPSTATUS;
					dbuf.used = 0;
				}
	 			else
				{
					state = WFS_INVALID;
					retval = WFR_INVALID;
					dbuf.used = 0;
				}
			}
			else {
				append_char(ch);
			}
			break;
			
		case WFS_INPUT:
			if(ch == 0x0d){
				state = WFS_CR;
			}
			else {
				append_char(ch);
			}
			break;
			
			
		default:
//			printf("INVALID\n");;
			
			state = WFS_INVALID;
			retval = WFR_INVALID;
			break;
	}
	
	return retval;
}

String WifiParser::get_ipaddr(){
	
	return  String((char *)ipaddr);
 }

String WifiParser::get_mac(){
	
	return  String((char *)mac);
}
 
String WifiParser::get_apip(){
	
	return  String((char *)apip);
 }

String WifiParser::get_apmac(){
	
	return  String((char *)apmac);
} 

bool WifiParser::get_ipd(ipd_data_t *ipd)
{
	if( state != WFS_IPD
		||  dbuf.used != dbuf.ipd.expect)
		return false;
		
	if(ipd){
		ipd->linkid 	= dbuf.ipd.linkid;
		ipd->portNum = dbuf.ipd.portNum;
		memcpy(ipd->ipaddr,  dbuf.ipd.ipaddr, 16);
		ipd->length = dbuf.used;
		ipd->data 	= dbuf.data;		// just copy pointer
	}
 
	//reset after getting IPD data
	this->reset();
 	return true;
}


bool WifiParser::get_data( uint8_t **outAllocData, size_t*  outSize){
 
	if(dbuf.dataLen == 0){
		return false;
 	};
	
	if(outAllocData){
		
		uint8_t* d = (uint8_t*) malloc(dbuf.dataLen + 1);
		if(d){
			memcpy(d, dbuf.data, dbuf.dataLen);
			*(d+dbuf.dataLen) = 0;  // null terminate
			*outAllocData = d;
		}
	}
 
	if(outSize){
		*outSize = dbuf.dataLen;
	}
	
	return true;
}

 
bool WifiParser::get_apInfo(ap_entry *apd)
{
	if(!hasAPD){
		return false;
	};
	
	if(apd){
		memcpy( apd->ssid, dbuf.apd.ssid ,  sizeof(dbuf.apd.ssid));
		memcpy(apd->mac, dbuf.apd.mac,  	 sizeof(dbuf.apd.mac));
		apd->ecn = dbuf.apd.ecn;
		apd->rssi = dbuf.apd.rssi;
		apd->channel = dbuf.apd.channel;
		apd->freq_offset = dbuf.apd.freq_offset;
		apd->freq_calibration = dbuf.apd.freq_calibration;
	}
	
	return true;
}


bool WifiParser::get_cipstatus(cipStatus_entry* cipStatus){
	
	if(!dbuf.dataLen)
		return false;

	cipStatus_entry cip;
	
	// CIPSTATUS:<link ID>,<type>,<remote IP>,<remote port>,<local port>,<tetype>

	char connType[4];
	
 	int result =  sscanf((const char *) dbuf.data, "%d,\"%3[^\"]\",\"%15[^\"]\",%d,%d,%d",
	 							&cip.linkid,
								connType,
								cip.ipaddr,
								&cip.remotePortNum,
								&cip.localPortNum,
								&cip.isServer );

	if(result < 6)
		return false;
	 
	if(strncmp(connType, "TCP", 3) == 0)
		cip.connType = CIPC_TCP;
	else if(strncmp(connType, "UDP", 3) == 0)
		cip.connType = CIPC_UDP;
	 else	if(strncmp(connType, "SSL", 3) == 0)
		 cip.connType = CIPC_SSL;
	else
		return false;
	
	if(cipStatus)
		*cipStatus = cip;
	
	return true;
}
