//
//  Utilities.h
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

#ifndef Utilities_h
#define Utilities_h

#include "CommonIncludes.h"

extern "C" {
 
void reconcileEvents(Stream *serial);
void processEvent(SavedEvent event, uint32_t when, Stream *serial);

bool isValidEvent(SavedEvent event);

const char* IC2DeviceName(uint8_t address);

void Serial_Printf(Stream *serial, const char *fmt, ...);
void Serial_Print(Stream *serial, const char *fmt);

void dumpHex(Stream *serial, uint8_t* buffer, int length, int offset);


} // extern c

#endif /* Utilities_h */