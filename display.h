//
//  display.h
//  clocktest
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

#ifndef display_h
#define display_h

#include <SerLCD.h> 			//  http://librarymanager/All#SparkFun_SerLCD

enum class BLColor {
	off 		= 0,
	red 		= 0xFF0000,
	orange 	= 0xFF8C00,
	yellow 	= 0xFFFF00,
	green 	= 0x00FF00,
	blue 		= 0x0000FF,
	indigo 	= 0x4B0082,
	violet	= 0xA020F0,
	grey		= 0x808080,
	white 	= 0xFFFFFF,
	dim		= 0x080808,
	invalid 	= 1,
};

class Display
{

public:
	
	Display();
	
  	bool begin(TwoWire &wirePort, uint8_t i2c_addr);
	
	void setBackLight(BLColor color);
	void setBackLightRGB(unsigned long rgb);
	
	BLColor stringToColor(const char* str);
	
	void updateDisplay(bool forceUpdate = false);

	void println(const char* str);
	void clear();
	
	bool hasDisplay;
	
private:
	
	SerLCD*		_lcd;
	uint32_t	 	_lastUpdate;
	bool 			_displayDimmed;
	
	void  init();
	
	void displayLinkStatus();
	void displayTemp();
	void displayClock();
	void displayState();
	void displayPowerMgmt();
 
};
#endif /* display_h */
