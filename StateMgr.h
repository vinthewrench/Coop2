//
//  StateMgr.hpp
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


#ifndef StateMgr_hpp
#define StateMgr_hpp

#include <stdio.h>


#ifdef  __cplusplus
extern "C" {
#endif


typedef enum  {
	STATE_UNKNOWN = 0,
	STATE_OPEN,
	STATE_OPENING,
	STATE_CLOSED,
	STATE_CLOSING,
	
	ANY_STATE  = 200, // used for state processing ..
	NO_NEW_STATE

}state_t;

typedef enum {
	
	EV_NONE = 0,
 
	EV_OPEN	= 1,
	EV_CLOSE = 2,
	
	EV_3		= 3,
	EV_4,
	EV_5,
	EV_6,
	EV_7,
	EV_8,
	EV_9
} event_t ;

 
class StateMgr {
	
public:
	
	StateMgr();
	
	void receive_event(event_t event );
	void loop();
	
	const char* stateText(state_t);
	
	state_t currentState;

private:
	
	typedef bool (*func_t)(StateMgr& self);
	
	typedef struct
	{
		state_t		 		current_state;
		event_t		 	 	received_event;
		func_t 				execute_function;
		state_t		 		next_state;
	} state_table_t;
	
	
	state_table_t*  state_table;
	unsigned int 	state_table_count ;
	
	struct Private;
	
};


#ifdef  __cplusplus
}
#endif

#endif /* StateMgr_hpp */
