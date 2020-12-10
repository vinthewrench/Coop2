//
//  telnetPortMgr.hpp
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


#ifndef remotePortMgr_hpp
#define remotePortMgr_hpp
 
#include <inttypes.h>
#include "LoopbackStream.h"
#include "CmdLineMgr.h"
#include "ESP8266Mgr.h"

#include "libtelnet.h"

class TelnetPortMgr;

typedef struct telnet_arg {
	TelnetPortMgr* 		_rp;
	telnet_t*				_tn;
	uint8_t 				_link_id;
	LoopbackStream*		_network;
	LoopbackStream* 		_in;
	LoopbackStream* 		_out;
	CmdLineMgr*	  		_cmdLn;
} telnet_arg_t;


class TelnetPortMgr {
	
public:

	TelnetPortMgr();
	bool begin(uint16_t portnum);
	void stop();
	
	int  activePortCount();
	
	bool closeRemoteConnection(uint8_t link_id);

	void loop();
	void broadcast(const char *fmt, ...);
	
	telnet_arg_t		_tnArg[MAX_WIFI_LINKS];
	
	bool _running;

private:
};



 
#endif /* remotePortMgr_hpp */
