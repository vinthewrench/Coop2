/* Copyright 2016 Pascal Vizeli <pvizeli@syshack.ch>
 * BSD License
 *
 * https://github.com/pvizeli/CmdParser
 */

#include "CmdBuffer.h"
 
const char*   CMDBUFFER_STR_PROMPT    = "> ";

bool CmdBufferObject::readFromSerial(Stream *serial, uint32_t timeOut)
{
    uint32_t isTimeOut;
    uint32_t startTime;
    bool     over = false;

    // UART initialize?
    if (serial == NULL) {
        return false;
    }

    ////
    // Calc Timeout
    if (timeOut != 0) {
        startTime = millis();
        isTimeOut = startTime + timeOut;

        // overloaded
        if (isTimeOut < startTime) {
            over = true;
        } else {
            over = false;
        }
    }

    ////
    // process serial reading
    do {

        // if data in serial input buffer
        while (serial->available()) {

            if (this->readSerialChar(serial)) {
                return true;
            }
        }

        // Timeout is active?
        if (timeOut != 0) {
            // calc diff timeout
            if (over) {
                if (startTime > millis()) {
                    over = false;
                }
            }

            // timeout is receive
            if (isTimeOut <= millis() && !over) {
                return false;
            }
        }

    } while (true); // timeout

    return false;
}

bool CmdBufferObject::readSerialChar(Stream *serial)
{
	uint8_t  readChar;
	uint8_t *buffer = this->getBuffer();
	
	// UART initialize?
	if (serial == NULL) {
		return false;
	}
	
	if (serial->available()) {
		// is buffer full?
		if (m_dataOffset >= this->getBufferSize()) {
			m_dataOffset = 0;
			return false;
		}
		
		// read into buffer
		readChar = serial->read();
		
		if (m_echo) {
			serial->write(readChar);
		}
		
		// is that the end of command
		if (readChar == m_endChar) {
			buffer[m_dataOffset] = '\0';
			m_dataOffset         = 0;
			return true;
		}
		
		// is that a backspace char?
		if (readChar == m_bsChar) {
			if(m_dataOffset > 0){
				--m_dataOffset;
				buffer[m_dataOffset] = '\0';
				
				if (m_echo) {
					serial->write(' ');
					serial->write(readChar);
				}
				return false;
			}
			else {
				this->clear();
				if (m_echo) {
					serial->write("\r\n");
				}
				return true;
			}
		}
		else  if (readChar == m_clearChar) {
			this->clear();
			if (m_echo) {
				serial->write(".....\r\n");
			}
			// return as if we actually read something.
			return true;
		}
		
		// is a printable character
		if (readChar > CMDBUFFER_CHAR_PRINTABLE) {
			buffer[m_dataOffset++] = readChar;
		}
	}
	return false;
}

void CmdBufferObject::displayPrompt(Stream *serial)
{
	serial->write(CMDBUFFER_STR_PROMPT);
}

 bool CmdBufferObject::hasDataInBuffer() 
 {
	 uint8_t *buffer = this->getBuffer();

	 return (buffer[0] != '\0');
 }
 
