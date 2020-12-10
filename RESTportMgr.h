//
//  RESTportMgr.hpp
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

#ifndef RESTportMgr_hpp
#define RESTportMgr_hpp

#include <stdio.h>
#include "http_parser.h"
#include "ESP8266Mgr.h"
#include "LoopbackStream.h"

class RESTportMgr;

typedef struct  {
	RESTportMgr* 		_rp;
	http_parser 			_parser;
	uint8_t 				_link_id;
	
	char* 					_url;		// must dealloc this
	char* 					_body;		// must dealloc this
 
	LoopbackStream*		_networkIn;
	LoopbackStream*		_networkOut;
	
} restPort_arg_t;

class RESTportMgr {
	
public:

	RESTportMgr();
	bool begin(uint16_t portnum);
	void stop();
	void shutdownLink(uint8_t link_id);

	int  activePortCount();

	void loop();
	
	bool _running;
 
	String statusJSON();
	String minstatusJSON();

	String setJSON(restPort_arg_t * arg);
	String doSync();
 
	restPort_arg_t 			_rpArg[MAX_WIFI_LINKS];
	http_parser_settings 	_hpCB;
	
private:
	
};

#endif /* RESTportMgr_hpp */
