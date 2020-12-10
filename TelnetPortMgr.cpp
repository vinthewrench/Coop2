//
//  telnetPortMgr.cpp
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

#include "TelnetPortMgr.h"

#include "CommonIncludes.h"
  
static const telnet_telopt_t RP_telopts[] = {
		{ TELNET_TELOPT_ECHO,      TELNET_WONT, TELNET_DONT },
 		{ TELNET_FLAG_NVT_EOL,     	TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_TTYPE,     TELNET_WONT, TELNET_DONT },
		{ TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DONT   },
		{ TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DONT   },
		{ TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DONT   },
		{ TELNET_TELOPT_BINARY,    TELNET_WONT, TELNET_DONT   },
		{ TELNET_TELOPT_NAWS,      TELNET_WONT, TELNET_DONT },
		{ -1, 0, 0 }
  };
 
static void telnet_event_handler(telnet_t *telnet, telnet_event_t *ev, void *arg) {
 
	(void)telnet;
	if(!arg) return;
	
	telnet_arg_t * tArg = (telnet_arg_t *) arg;
	TelnetPortMgr*  _rp = tArg->_rp;
	
	if(!_rp->_running)
		return;
 
	switch (ev->type) {
			
	case TELNET_EV_DATA:
			
//			Serial_Printf(&Serial, "RCV [%zi] ==> ", ev->data.size);
//			dumpHex(&Serial,(unsigned char*)  ev->data.buffer, (int) ev->data.size, 0);

			tArg->_in->write((const uint8_t*) ev->data.buffer,
							  ev->data.size);
		break;
			
	case TELNET_EV_SEND:
			
//			Serial_Printf(&Serial, "SEND [%zi] ==> ", ev->data.size);
//			dumpHex(&Serial, (unsigned char*) ev->data.buffer, (int) ev->data.size,0);

			tArg->_network->write((const uint8_t*) ev->data.buffer, ev->data.size);
		break;
	
		case TELNET_EV_WARNING:
			Serial_Printf(&Serial, "WARNING: %s\n", ev->error.msg);
			break;
			
		case TELNET_EV_ERROR:
			Serial_Printf(&Serial,  "ERROR: %s\n", ev->error.msg);
			break;
			
		default:
			break;
	}
}


void telnetServerCallback(uint8_t 		link_id,
						  void*		arg,
						  int  		status,
						  char			ipaddr[16],
						  int 			portNum,
					const uint8_t* 	data,
						  size_t		dataLen){
	
	TelnetPortMgr* rp = (TelnetPortMgr*) arg;

	if(!rp->_running)
		return;

	if(status == WFCF_CLOSED){
	
		cmdLineMgr.print("REMOTE CLOSE [%d] ",   link_id);
	
//		Serial_Printf(&Serial, "SERVER CLOSED [%d] ",link_id );

		telnet_arg_t * arg =  &rp->_tnArg[link_id];
		arg->_link_id = MAX_WIFI_LINKS;
 		arg->_cmdLn->stop();
 		delete arg->_cmdLn; 			arg->_cmdLn = NULL;
 //		telnet_free(arg->_tn);	 	arg->_tn = NULL;
		delete arg->_in;  				arg->_in = NULL;
		delete arg->_out;				arg->_out = NULL;
//		delete arg->_network;			arg->_network = NULL;

//		Serial_Printf(&Serial, "SERVER CLOSED2  [%d] ",link_id );

	}
	else if(status == WFCF_OPEN){
		
		cmdLineMgr.print("REMOTE OPEN [%d] %s %d",   link_id, ipaddr, portNum);
		
//		Serial_Printf(&Serial, "SERVER OPEN [%d] %s %d",
//						  link_id,
//						  ipaddr, portNum);
//
		
		telnet_arg_t * arg =  &rp->_tnArg[link_id];
		arg->_rp = rp;
		arg->_link_id = link_id;
		arg->_in = new LoopbackStream(1024);
		arg->_out = new LoopbackStream(1024);
		arg->_network = new LoopbackStream(1024);
		arg->_tn = telnet_init(RP_telopts, telnet_event_handler,  0, arg);
	
		telnet_negotiate(arg->_tn, TELNET_WONT, TELNET_TELOPT_ECHO);
		telnet_negotiate(arg->_tn, TELNET_WILL, TELNET_FLAG_NVT_EOL);

		arg->_cmdLn = new CmdLineMgr();
		arg->_cmdLn->begin(arg->_in, arg->_out, link_id);
	 
	}
	else if(status == WFCF_IPD){
 
		telnet_arg_t * arg =  &rp->_tnArg[link_id];
//
//
//		Serial_Printf(&Serial, "SERVER RECV [%d] len:%ld, %s %d",
//						  link_id,
//						  dataLen,
//						  ipaddr, portNum);
//
//		dumpHex(&Serial,(unsigned char*) data, (int) dataLen, 0);
//
		telnet_recv(arg->_tn, (const char *) data,dataLen);
		
	}
	else
	{
		Serial_Printf(&Serial, "SERVER CALLBACK [%d] ",link_id);
	}
}

TelnetPortMgr::TelnetPortMgr() {
	
	_running = false;
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		_tnArg[idx]._link_id = MAX_WIFI_LINKS;
	}
	
};


bool TelnetPortMgr::begin(uint16_t portnum){
	_running = false;
 
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		_tnArg[idx]._link_id = MAX_WIFI_LINKS;
	}
	
//	_stream.clear();
	
	if(!wifiMgr.hasInterNet) {
//		if(! wifiMgr.start(&Serial)) {
			return false;
//	 	}
	}
 
	if(!wifiMgr.startServer(portnum, telnetServerCallback, this)) {
		return false;
	}
 
	const char *strIP = wifiMgr.ipAddr();
 
	Serial_Printf(&Serial, " Remote Commandline started. %s port %u ",
						strIP, portnum );
 
	_running = true;
	return true;
}

void TelnetPortMgr::stop()
{
	
	if(!_running)
		return;
	
	_running = false;

	wifiMgr.stopServer();
 
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		
		telnet_arg_t * arg = &_tnArg[idx];
		if(arg->_link_id != MAX_WIFI_LINKS){
			
			arg->_cmdLn->stop();
			delete arg->_cmdLn;
			arg->_cmdLn = NULL;
			arg->_link_id = MAX_WIFI_LINKS;
//			telnet_free(arg->_tn); 	arg->_tn = NULL;
			delete arg->_in;
			delete arg->_out;
 			delete arg->_network;
		}
	}
	
	Serial_Printf(&Serial," Remote Commandline stopped");
}

void TelnetPortMgr::broadcast(const char *fmt, ...){
	
	if(!_running)
		return;
	
 	va_list args;
	va_start(args, fmt);
	
	char data[1024];

	vsprintf(data, fmt, args);
	size_t dataLen = strlen(data) + 2;
 
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		
		telnet_arg_t * arg =  &_tnArg[idx];
		if(arg->_link_id != MAX_WIFI_LINKS){

//			Serial_Printf(&Serial, "BROADCAST[%u] - (%d)|%s| ", arg->_link_id, dataLen, data);

			telnet_send(arg->_tn, (const char *) data,dataLen);
		}
 	}
	va_end(args);
}

bool TelnetPortMgr::closeRemoteConnection(uint8_t link_id){
	
	if(!_running)
		return false;

	telnet_arg_t * arg = &_tnArg[link_id];

	if(arg->_link_id == MAX_WIFI_LINKS)
		return false;

 	arg->_cmdLn->stop();
	
//	delete arg->_cmdLn;
//	arg->_cmdLn = NULL;
//	arg->_link_id = MAX_WIFI_LINKS;
////			telnet_free(arg->_tn); 	arg->_tn = NULL;
//	delete arg->_in;
//	delete arg->_out;
////			delete arg->_network;


	return wifiMgr.closeConnection(link_id);
	
	return true;
}

void TelnetPortMgr::loop(){
	if(!_running)
		return;
	
	for(int idx = 0; idx < MAX_WIFI_LINKS; idx++ ){
		
		telnet_arg_t * arg =  &_tnArg[idx];
		if(arg->_link_id != MAX_WIFI_LINKS){
			
			arg->_cmdLn->loop();
			
			while(arg->_out->available() > 0){
				
				size_t dataLen = arg->_out->available();
				char data[128];
				
				if(dataLen > sizeof(data))
					dataLen = sizeof(data);
				
				for(int i = 0 ; i < dataLen ;i++){
					data[i] = arg->_out->read();
				}
				
				telnet_send(arg->_tn, data, dataLen);
			}
			
			
			while(arg->_network->available() > 0){
				
				size_t dataLen = arg->_network->available();
				char data[128];
				
				if(dataLen > sizeof(data))
					dataLen = sizeof(data);
				
				for(int i = 0 ; i < dataLen ;i++){
					data[i] = arg->_network->read();
				}
				
				wifiMgr.sendData(arg->_link_id, (const uint8_t* )data,dataLen);
			}
			
		}
	}
	
	
}


int TelnetPortMgr::activePortCount(){
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
