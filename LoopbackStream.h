

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

#pragma once

#include <Stream.h>

/*
 * A LoopbackStream stores all data written in an internal buffer and returns it back when the stream is read.
 * 
 * If the buffer overflows, the last bytes written are lost.
 * 
 * It can be used as a buffering layer between components.
 */

class LoopbackStream : public Stream {
	uint8_t *buffer;
	uint16_t buffer_size;
	uint16_t pos, size;
public:
	
	static const uint16_t DEFAULT_SIZE = 256;
	
	bool done;		// useful flag to incocate we are done with stream
	
	LoopbackStream(uint16_t buffer_size = LoopbackStream::DEFAULT_SIZE);
	~LoopbackStream();
	
	/** Clear the buffer */
	void clear();
	
	virtual size_t write(const uint8_t*, size_t);
	
	virtual size_t write(uint8_t);
	virtual int availableForWrite(void);
	
	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
};
