//
//  buttons.hpp
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


#ifndef buttons_hpp
#define buttons_hpp

#include <SparkFun_Qwiic_Button.h>


typedef enum {
	BUTTON_GREEN = 0,
	BUTTON_RED,
}button_t;


class Buttons
{
public:
	Buttons();
	bool begin(button_t, uint8_t address);
	void LEDoff(button_t);
	void LEDon(button_t);
	void LEDblink(button_t);
	void loop();

	bool isConnected(button_t);

	bool self_test();

private:
	
	QwiicButton* buttonForID(button_t);

	QwiicButton* _green;
	QwiicButton*  _red;

};

#endif /* buttons_hpp */
