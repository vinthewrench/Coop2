//
//  WifiParser.hpp
 

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

#ifndef WifiParser_hpp
#define WifiParser_hpp

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <stdio.h>

#include <Arduino.h>

typedef enum {
	WFR_INVALID = 0,
	WFR_OK,
	WFR_ERROR,
	WFR_FAIL,
	WFR_WIFI_CONNECTED,
	WFR_WIFI_GOT_IP,
	WFR_DISCONNECT,
	WFR_BUSY_S,
	WFR_BUSY_P,
	WFR_CONNECT,
	WFR_CLOSED,
	WFR_IPD,
	WFR_SEND_OK,
	WFR_SEND_FAIL,
	WFR_UNLINK,
	
	WFR_STAIP,
	WFR_STAMAC,
	WFR_APIP,
	WFR_APMAC,

	WFR_CWLAP,
	WFR_CJLAP,  // join AP error or info
	WFR_CIPSTATUS, // +CIPSTATUS:0,"UDP","107.170.0.6",123,1356,0

	WFR_SEND_DATA,		// received > waitingh for data
	WFR_OTHER,
	WFR_READY,
 
	WFR_CONTINUE,

	WFR_NOTHING,		// we never return this.  use it for Stream->available() == 0

}wf_resulr_t;


typedef struct  {
	int 			linkid;
	char			ipaddr[16];
	int 			portNum;
	
	uint8_t*  	data;
	size_t			length;
}ipd_data_t;

typedef struct  {
	char	ssid[33];
	char	mac[18];
	int 	ecn;  // encyption method
	/* ‣ 0: OPEN
	 ‣ 1: WEP
	 ‣ 2: WPA_PSK
	 ‣ 3: WPA2_PSK
	 ‣ 4: WPA_WPA2_PSK
	 ‣ 5: WPA2_Enterprise (AT can NOT connect to WPA2_Enterprise AP for now.)
	 */
	
	int 	rssi; // signal strength.
	int 	channel; // channel number
	int 	freq_offset; // frequency offset of AP; unit: KHz.
	int 	freq_calibration; //  calibration for frequency offset.
} 	ap_entry;


typedef enum {
	CIPC_INVALID = 0,
	CIPC_UDP,
	CIPC_TCP,
 	CIPC_SSL,
}CIPC_t;

typedef struct  {
	int 			linkid;
	CIPC_t 		connType;		// TCP, UDP, SSL
	char			ipaddr[16];
	int 			remotePortNum;
	int 			localPortNum;
	int 			isServer;
} cipStatus_entry;


class WifiParser {
	
public:
	
	WifiParser();
	~WifiParser();
	
	void  reset();
	
	wf_resulr_t process_char(uint8_t ch);
	
	String get_ipaddr();
	String get_mac();
	String get_apip();
	String get_apmac();

	bool get_ipd(ipd_data_t *ipd);
	bool get_apInfo(ap_entry* ap_entry);

	bool get_cipstatus(cipStatus_entry* cipStatus);

	bool get_data( uint8_t **outAllocData, size_t*  outSize);
 
private:
	
	void dbuff_init();
	bool dbuff_cmp(const char* str);
	bool dbuff_cmp_end(const char* str);
	
	bool append_data(void* data, size_t len);
	bool append_char(uint8_t c);
	size_t data_size();
	
	struct  {
		size_t  	used;			// actual end of buffer
		size_t  	dataLen;		// data len
		size_t  	alloc;
		uint8_t*  data;
		
		union {
			struct {	// for +IPD
				size_t 		expect;
				int 			linkid;
				char			ipaddr[16];	// null terminated
				int 			portNum;
			} ipd;
			
			ap_entry apd;
		};
	} dbuf;
	
	typedef enum  {
		WFS_UNKNOWN = 0,
		WFS_START,
		WFS_PLUS,
		WFS_CR,
		WFS_INPUT,
		
		WFS_IPD,
		WFS_CIFSR,
		WFS_CWLAP,
		WFS_CIPSTATUS, // status info
		WFS_CJLAP,  // join AP error

		WFS_SENDING, // waiting on send data

		WFS_INVALID,
	}wf_state_t;
	 
	
	const char * sReady  			= "ready";
	const char * sOK  				= "OK";
	const char * sERROR   			= "ERROR";
	const char * sFAIL   			= "FAIL";
	const char * sUNLINK   		= "UNLINK";

	const char * sWifiConnect   	= "WIFI CONNECTED";
	const char * sWifiDisconnect = "WIFI DISCONNECT";
	const char * sWifiGotIP 		= "WIFI GOT IP";
	const char * sBusyS				= "busy s...";
	const char * sBusyP				= "busy p...";
	
	const char * sSendOK			= "SEND OK";
	const char * sSendFail			= "SEND FAIL";

	const char * sConnect			= "CONNECT";
	const char * sClosed			= "CLOSED";

	const char * sCIFSR 			= "+CIFSR";
	const char * sIPD 			   = "+IPD";
	const char * sCWLAP 			= "+CWLAP";
	const char * sCWJAP 			= "+CWJAP";
	const char * sCIPSTATUS 		= "+CIPSTATUS";

	bool 	hasIP;
	bool	hasMAC;
	bool	hasAPMAC;
	bool	hasAPIP;

	bool	hasAPD;

	char	ipaddr[17];
	char	mac[18];
	char	apip[17];
	char	apmac[18];
 
	wf_state_t 	state;
};



#endif /* WifiParser_hpp */
