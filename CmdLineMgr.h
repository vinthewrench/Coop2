//
//  CmdLineMgr.hpp
//  chickencoopArt
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


#ifndef CmdLineMgr_hpp
#define CmdLineMgr_hpp

#include <stdint.h>
#include <string.h>

#include <Arduino.h>

#include "CmdBuffer.h"
#include "CmdCallback.h"
#include "CmdParser.h"
#include "ESP8266Mgr.h"
#include "LoopbackStream.h"


extern "C++" {
class CmdLineMgr {
	
public:
	
	CmdLineMgr();
	
	void begin(Stream *input ,Stream *output, uint8_t link_id = MAX_WIFI_LINKS);
	void stop();

	void loop();
	
	bool isBusy();
	
	void print(const char *fmt, ...);
	
	CmdCallback<20> cmdCallback;
	CmdBuffer<64> cmdBuffer;
	CmdParser     cmdParser;

	///
	Stream*  		_serialIn;
	Stream*  		_serialOut;
	uint8_t 		_link_id;
	
	LoopbackStream* 		_out;
 
private:
	
	bool isSetup;
};

};
 
#endif /* CmdLineMgr_hpp */
