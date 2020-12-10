//
//  Utilities.c
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

#include "Utilities.h"
#include "CommonIncludes.h"
 
extern "C" {


typedef struct{
	uint8_t address;
	const char*   description;
}  Ic2Tab_entry;

const Ic2Tab_entry IC2_Table[] =
{
	{0x48,"TMP102"},
	{0x49,"TMP102"},
	{0x4A,"TMP102"},
	{0x4B,"TMP102"},
	{0x69,"RV1805"},
	{0x6D,"Relay"},
	{0x6e,"QwiicButton Red"},
	{0x6F,"QwiicButton Green"},
	{0x72, "LCD"},
	{0x4F, "LCD"}
};

const char* IC2DeviceName( uint8_t address){
	
	size_t max_device = sizeof(IC2_Table)/ sizeof(Ic2Tab_entry);
	
	for(int i = 0; i < max_device; i++){
		Ic2Tab_entry entry = IC2_Table[i];
		if(address == entry.address){
			return entry.description;
		}
	}
	
	return NULL;
}
 

void Serial_Print(Stream *serial, const char *fmt){
	
	serial->write(fmt);
	serial->write('\n');
	
	//	serial->println(fmt);
}


void Serial_Printf(Stream *serial, const char *fmt, ...)
{
	if(!serial) return;
	
	char buff[1024];
	
	va_list args;
	va_start(args, fmt);
	
	vsprintf(buff, fmt, args);
	
	serial->write(buff);
	serial->write('\n');
	//		serial->println(buff);
	serial->flush();
	
	va_end(args);
}



void dumpHex(Stream *serial, uint8_t* buffer, int length, int offset)
{
	char hexDigit[] = "0123456789ABCDEF";
	register int			i;
	int						lineStart;
	int						lineLength;
	short					c;
	const unsigned char	  *bufferPtr = buffer;
	
	char                    lineBuf[1024];
	char                    *p;
	
#define kLineSize	8
	for (lineStart = 0, p = lineBuf; lineStart < length; lineStart += lineLength,  p = lineBuf )
	{
		lineLength = kLineSize;
		if (lineStart + lineLength > length)
			lineLength = length - lineStart;
		
		p += sprintf(p, "%6d: ", lineStart+offset);
		for (i = 0; i < lineLength; i++){
			*p++ = hexDigit[ bufferPtr[lineStart+i] >>4];
			*p++ = hexDigit[ bufferPtr[lineStart+i] &0xF];
			if((lineStart+i) &0x01)  *p++ = ' ';  ;
		}
		for (; i < kLineSize; i++)
		p += sprintf(p, "   ");
		
		p += sprintf(p,"  ");
		for (i = 0; i < lineLength; i++) {
			c = bufferPtr[lineStart + i] & 0xFF;
			if (c > ' ' && c < '~')
				*p++ = c ;
			else {
				*p++ = '.';
			}
		}
		*p++ = 0;
		
		
		Serial_Printf(serial, "%s",lineBuf);
	}
#undef kLineSize
}

//#if DEBUG_PRINT

void debug_printf(const char *fmt, ...) {
	
	char buff[256];
	
	va_list args;
	va_start(args, fmt);
	
	vsprintf(buff, fmt, args);
	Serial.println(buff);
	
	va_end(args);
}
//#endif


void processEvent(SavedEvent event, uint32_t when, Stream *serial){
	
	cmdLineMgr.print("EVT %s %02d:%02d %s", evenStr[event],
						  hourFormat12(when),minute(when), isAM(when)?"AM":"PM");
	
	switch(event){
		case SE_LIGHT1_ON:
			door.setLight1(true);
			break;
			
		case SE_LIGHT1_OFF:
			door.setLight1(false);
			break;
			
		case SE_RELAY4_ON:
			door.setRelay4(true);
			break;
			
		case SE_RELAY4_OFF:
			door.setRelay4(false);
			break;
			
		case SE_DOOR_OPEN:
			stateMgr.receive_event( EV_OPEN);
			break;
			
		case SE_DOOR_CLOSE:
			stateMgr.receive_event( EV_CLOSE);
			break;
			
		default: break;
	}
	delay(100);
	
}

void reconcileEvents(Stream *serial) {
	
	SavedEvent eventList[16] = { SE_NONE };
	size_t  count;
	time_t  date =  tsMgr.timeNow();
	
	if( scdMgr.reconcileEvents(eventList,
										sizeof(eventList) / sizeof (SavedEvent),
										&count )){
		
		
		for(int i = 0; i < count; i++){
			
			SavedEvent event = eventList[i];
			processEvent(event, date, serial);
		}
	}
}


bool isValidEvent(SavedEvent event){
	return ((event >= SE_NONE) && (event < SE_INVALID));
}


} // extern c

