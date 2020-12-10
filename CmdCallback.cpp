/* Copyright 2016 Pascal Vizeli <pvizeli@syshack.ch>
 * BSD License
 *
 * https://github.com/pvizeli/CmdParser
 */

#include "CmdCallback.h"

#define ENTER_CALLBACK _busyInCallback = true;
#define EXIT_CALLBACK 	_busyInCallback = false;

void CmdCallbackObject::loopCmdProcessing(CmdParser *      cmdParser,
                                          CmdBufferObject *cmdBuffer,
                                          Stream *         serial)
{
    do {
        // read data
        if (cmdBuffer->readFromSerial(serial)) {

            // parse command line
            if (cmdParser->parseCmd(cmdBuffer) != CMDPARSER_ERROR) {
                // search command in store and call function
                // ignore return value "false" if command was not found
					ENTER_CALLBACK;
                this->processCmd(cmdParser);
					EXIT_CALLBACK;
                cmdBuffer->clear();
					
            }
        }
    } while (true);
}

bool CmdCallbackObject::processCmd(CmdParser *cmdParser)
{
	bool status = false;
    char *cmdStr = cmdParser->getCommand();

    // check is command is  okay
    if (cmdStr == NULL) {
        return false;
    }

    // search cmd in store
    for (size_t i = 0; this->checkStorePos(i); i++) {

        // compare command with string
        if (this->equalStoreCmd(i, cmdStr)) {
            // call function
            status =  this->callStoreFunct(i, cmdParser);
			  break;
        }
    }

    return status;
}

bool CmdCallbackObject::updateCmdProcessing(CmdParser *      cmdParser,
                                            CmdBufferObject *cmdBuffer,
                                            Stream *         serial)
{
	// read data and check if command was entered
	
	bool didRead = cmdBuffer->readSerialChar(serial);
	
	if (didRead) {
		
		// we check if actual characters are in the buffer -- we could
		// simply have zero out the line.
		
		if(cmdBuffer->hasDataInBuffer()){
			
			// parse command line
			if (cmdParser->parseCmd(cmdBuffer) != CMDPARSER_ERROR) {
				// search command in store and call function
				// ignore return value "false" if command was not found
				
				ENTER_CALLBACK;
				if(! this->processCmd(cmdParser)) {
					if(_errCB != NULL) {
						_errCB(_cBArg, cmdBuffer);
					}
				}
				EXIT_CALLBACK
				
			}
		}
		
		cmdBuffer->clear();

		if(_compCB != NULL) {
			_compCB(_cBArg, cmdParser);
		}
		
	}
	return didRead;
}


bool CmdCallbackObject::hasCmd(char *cmdStr)
{
    // search cmd in store
    for (size_t i = 0; this->checkStorePos(i); i++) {

        // compare command with string
        if (this->equalStoreCmd(i, cmdStr)) {
            return true;
        }
    }

    return false;
}

  void CmdCallbackObject::addErrorHandler(ErrCallFunct errFunct)
  {
    _errCB = errFunct; 
  }
 

void CmdCallbackObject::addCompletionHandler(CmdCallFunct compFunct)
{
	_compCB = compFunct;
}

