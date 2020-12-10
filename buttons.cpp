//
//  buttons.cpp
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


#include "buttons.h"
#include "CommonIncludes.h"

Buttons::Buttons(){
	_green = NULL;
	_red = NULL;
}

bool Buttons::begin(button_t buttonID, uint8_t address){
	
	QwiicButton* button = new QwiicButton();
	
	if(button->begin(address)){
		
		switch (buttonID) {
			case BUTTON_GREEN:
				_green = button;
 				_green->LEDoff();
	 			return true;
				break;
				
			case BUTTON_RED:
				_red = button;
 				_red->LEDoff();
				return true;
				break;
				
			default:
				return false;
				break;
		}
	}
	
	return false;
}

QwiicButton* Buttons::buttonForID(button_t buttonID){
	switch (buttonID) {
		case BUTTON_GREEN:
			return _green;
 
		case BUTTON_RED:
			return _red;
			
		default:
			return NULL;
	}
}
	
void Buttons::LEDoff(button_t buttonID){
	 
	QwiicButton* button = this->buttonForID(buttonID);
	if(button)
		button->LEDoff();
	
}
void Buttons::LEDon(button_t buttonID){

	QwiicButton* button = this->buttonForID(buttonID);
	if(button)
		button->LEDon();
}

void Buttons::LEDblink(button_t buttonID){

	QwiicButton* button = this->buttonForID(buttonID);
	if(button)
		button->LEDconfig(100, 500, 100);

}

void Buttons::loop(){

 
	if (_green && _green->isPressed()) {
		
		stateMgr.receive_event(EV_OPEN);
		
		while(_green->isPressed() == true) {
			stateMgr.receive_event( EV_NONE);
			
			display.updateDisplay(true);
			delay(10);  //wait for user to stop pressing
		}
	}
	
	if (_red && _red->isPressed()) {
	 
		stateMgr.receive_event(EV_CLOSE);
		
		while(_red->isPressed() == true) {
			stateMgr.receive_event( EV_NONE);
			
			display.updateDisplay(true);
			delay(10);  //wait for user to stop pressing
		}
	}

}


bool Buttons::isConnected(button_t buttonID){
	QwiicButton* button = this->buttonForID(buttonID);
	return(button != NULL);
}

bool Buttons::self_test(){
	
	for(int i = 0; i < 3;  i++){
		this->LEDon(BUTTON_GREEN);
		delay(200);
		this->LEDoff(BUTTON_GREEN);
		delay(200);
		this->LEDon(BUTTON_RED);
		delay(200);
		this->LEDoff(BUTTON_RED);
		delay(200);
	}

	return (_green != NULL) && ( _red != NULL); 
}
