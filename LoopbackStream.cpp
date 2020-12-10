
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


#include "LoopbackStream.h"
#include "CommonIncludes.h"

LoopbackStream::LoopbackStream(uint16_t buffer_size) {
  this->buffer = (uint8_t*) malloc(buffer_size);
  this->buffer_size = buffer_size;
  this->pos = 0;
  this->size = 0;
	this->done = false;
}

LoopbackStream::~LoopbackStream() {
	
	if(this->buffer)
 	 free(this->buffer);
}

void LoopbackStream::clear() {
  this->pos = 0;
  this->size = 0;
}

int LoopbackStream::read() {
  if (size == 0) {
    return -1;
  } else {
    int ret = buffer[pos];
    pos++;
    size--;
    if (pos == buffer_size) {
      pos = 0;
    }
    return ret;
  }
}

size_t LoopbackStream::write(const uint8_t* data, size_t dataLen){
	
	if (size == buffer_size) {
	  return 0;
	}
	if( dataLen > this->availableForWrite() ){
		return 0;
	}
	
	for(size_t len = 0; len < dataLen; len++)
	this->write(data[len]);
 
	return 1;
}


size_t LoopbackStream::write(uint8_t v) {
  if (size == buffer_size) {
    return 0;
  } else {
    int p = pos + size;
    if (p >= buffer_size) {
      p -= buffer_size;
    }
    buffer[p] = v;
    size++;
    return 1;
  }  
}

int LoopbackStream::available() {
  return size;
}

int LoopbackStream::availableForWrite() {
  return buffer_size - size;
}

int LoopbackStream::peek() {
  return size == 0 ? -1 : buffer[pos];
}

void LoopbackStream::flush() {
  //I'm not sure what to do here...
}

