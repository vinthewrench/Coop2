//
//  StateMgr.cpp
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


#include "StateMgr.h"
#include "CommonIncludes.h"


unsigned long  open_time = 0;		// for tracking open door delay
unsigned long  close_time = 0;		// for tracking open door delay


const unsigned long  open_delay  = 15000;
const unsigned long  close_delay  = 15000;


struct StateMgr::Private
{
	
	static bool Null_Action(StateMgr& self)
	{
		return true;
		
	}
		
	static bool Open_Action(StateMgr& self)
	{
		
		// this is a NOP
		if((self.currentState == STATE_OPEN)
			|| (self.currentState == STATE_OPENING))
			return true;
		
		door.stop();
		door.open();
		open_time = millis();
		self.currentState = STATE_OPENING;
		return true;
	}
	
	static bool Opening_Transition(StateMgr& self)
	{
		if((millis() - open_time) >= open_delay) {
			door.stop();
			self.currentState = STATE_OPEN;
			return true;
		}
		return false;
		
	}
	
	static bool Close_Action(StateMgr& self)
	{
	
		// this is a NOP
		if((self.currentState == STATE_CLOSED)
			|| (self.currentState == STATE_CLOSING))
			return true;
		
		door.stop();
		door.close();
		close_time = millis();
		self.currentState = STATE_CLOSING;
		return true;
		
	}
	
	static bool Closing_Transition(StateMgr& self)
	{
		if((millis() - close_time) >= close_delay) {
			door.stop();
			self.currentState = STATE_CLOSED;
			return true;
		}
		else if(door.coopDoorisClosed()) {
			door.stop();
			self.currentState = STATE_CLOSED;
			return true;
		}
//		else if(door.safetySwitch()) {
//			door.stop();
//			Open_Action(self);
//			self.currentState = STATE_OPENING;
//			return true;
//		}
		
		return false;
	}
	
	
	static void receive_event_internal(StateMgr& self,  event_t event ){

//		if(event != EV_NONE) DPRINTF("RCV %u", event );
		
		const int STATE_TABLE_ENTRIES=   self.state_table_count;
		const  state_table_t * thisTable =  self.state_table;
		
		for (int  i = 0; i < STATE_TABLE_ENTRIES; i++ )
		{
			if ( ( event == thisTable->received_event ) &&
				 ( ( self.currentState == thisTable->current_state ) ||
				  ( thisTable->current_state == ANY_STATE ) ) )
			{
				if (( *thisTable->execute_function)(self) == true) {
					if ( thisTable->next_state != NO_NEW_STATE )
					{
						self.currentState = thisTable->next_state;
					}
					break;
				}
			}
			thisTable++;
		}
	};
	
};


void StateMgr::receive_event(event_t event ){
	Private::receive_event_internal(*this, event);
};

void StateMgr::loop(){
	Private::receive_event_internal(*this, EV_NONE);
}

const char* StateMgr::stateText(state_t state){
	
	const char* STR_UNKNOWN = 	"Unknown";
	const char* STR_OPEN = 		"Open";
	const char* STR_CLOSED = 	"Closed";
	const char* STR_OPENING = 	"Opening";
	const char* STR_CLOSING = 	"Closing";


	const char *str = STR_UNKNOWN;
	
	switch(state) {
		case STATE_UNKNOWN:
			str = STR_UNKNOWN;
			break;
			
		case STATE_OPEN:
			str = STR_OPEN;
			break;
			
		case STATE_OPENING:
			str = STR_OPENING;
			break;
			
		case STATE_CLOSED:
			str = STR_CLOSED;
			break;
			
		case STATE_CLOSING:
			str = STR_CLOSING;
			break;
	}

	return str;
	
}


StateMgr::StateMgr(){
	
	static  state_table_t  tbl[] = {
		
		{STATE_OPENING , EV_NONE, &Private::Opening_Transition, NO_NEW_STATE},
		{STATE_CLOSING , EV_NONE, &Private::Closing_Transition, NO_NEW_STATE},
			
		{ANY_STATE , EV_OPEN,  &Private::Open_Action, NO_NEW_STATE},
		{ANY_STATE , EV_CLOSE, &Private::Close_Action, NO_NEW_STATE},
		
		{ANY_STATE , EV_NONE, &Private::Null_Action, NO_NEW_STATE},
		
	};
	
	
	state_table = tbl;
	state_table_count  = sizeof(tbl) / sizeof(state_table_t);
}


