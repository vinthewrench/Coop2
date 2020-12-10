
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


#include "CommonIncludes.h"
#include "TimeLib.h"
 

/*
 
 I2C device found at address 0x48  !  TMP102
 I2C device found at address 0x69  !  RV1805
 I2C device found at address 0x6D  !  Relay
 I2C device found at address 0x6e  !  QwiicButton Red
 I2C device found at address 0x6F  !  QwiicButton Green
 I2C device found at address 0x72 /0x4F !  LCD
 */


#define IC2_ADDR_LCD 			0x72
#define IC2_ADDR_RELAY 		0x6D
#define IC2_ADDR_BUTTON_GREEN 0x6F
#define IC2_ADDR_BUTTON_RED 	0x6E

constexpr uint8_t pin_esp8266_reset = 2; // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

constexpr uint8_t pin_ACpower			 = 5; 	//  from optocoupler indicating power fail
constexpr uint8_t pin_enclosureDoor = 6; 	// the pin that the encloure door is connected to
constexpr uint8_t pin_coopDoorClosed = 7; //  grounded when coop door is closed
 
const char* 		versionString	= "1.0.b1";

WifiMgr			wifiMgr;
APM3_RTC      	rtc;
TimeSyncMgr  	tsMgr;
EEPPROMMgr    	eeMgr;
ScheduleMgr   	scdMgr;
StateMgr			stateMgr;
CmdLineMgr		cmdLineMgr;
TelnetPortMgr	telnetPortMgr;
RESTportMgr		restPortMgr;

TempSensor 		tempSensor;

PowerMgr 			powerMgr;
Display 			display;
Buttons			buttons;
Door				door;

// Watchdog timer configuration structure.
am_hal_wdt_config_t g_sWatchdogConfig = {

  // Configuration values for generated watchdog timer event.
  .ui32Config = AM_HAL_WDT_LFRC_CLK_16HZ | AM_HAL_WDT_ENABLE_RESET /*| AM_HAL_WDT_ENABLE_INTERRUPT */,

  // Number of watchdog timer ticks allowed before a watchdog interrupt event is generated.
//  .ui16InterruptCount = 240, // Set WDT interrupt timeout for 10 seconds (80 / 16 = 5).

  // Number of watchdog timer ticks allowed before the watchdog will issue a system reset.
  .ui16ResetCount = 1000 // Set WDT reset timeout for 30 seconds (480 / 16 = 15).
};

 
bool setupComplete = false;
bool hasTemp = false;
bool eventsAreReconciled = false;
bool forceSync = false;
bool eventsAreSetup = true;
bool cmdLineisSetup = false;
bool networkisSetup = false;
 
void setup() {

	// for debugging
	Serial.begin(230400);
	while (!Serial);
	DPRINTF("Starting Wifi on Artemis testing");
 
	Wire.begin();
	Wire.setClock(400000); //Optional - set I2C SCL to High Speed Mode of 400kHz

	// why did we start?  was it the watchdog?
	
	//  // (Note: See am_hal_reset.h for RESET status structure)
	am_hal_reset_status_t sStatus;
	am_hal_reset_status_get(&sStatus);
	
	// for debugging
	if(sStatus.bPORStat) DPRINTF("Power-On reset");
	else if (sStatus.bEXTStat) DPRINTF("External reset");
	else if (sStatus.bWDTStat) DPRINTF("Watch Dog Timer reset");
	else  DPRINTF ("Reset Status Register = 0x%x", sStatus.eStatus);
	// for debugging
 
// ---  setup watchdog timer

		// Set the clock frequency.
		am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

	// Set the default cache configuration
	  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	  am_hal_cachectrl_enable();

	// Clear reset status register for next time we reset.
	  am_hal_reset_control(AM_HAL_RESET_CONTROL_STATUSCLEAR, 0);

	  // LFRC must be turned on for this example as the watchdog only runs off of the LFRC.
	  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

	  // Configure the watchdog.
	  am_hal_wdt_init(&g_sWatchdogConfig);

	  // Enable the watchdog.
	  am_hal_wdt_start();
// ---
	
	// hook up display
	display.begin(Wire,IC2_ADDR_LCD);

// hook up the wifi device
	Serial1.begin(115200);
	while (!Serial1);
	wifiMgr.begin(&Serial1, pin_esp8266_reset);
	
// make sure EEPROM IS SETUP before using any values
	if(!eeMgr.isSetup()){
		display.println("EEPROM NOT SETUP");
 		eeMgr.clear_all();
		
	//MARK: remove before SHIP
		int    gmtOffset =  -8;     // PST
		double longitude =  -122.865947;
		double latitude  =  42.235389;

		eeMgr.set_timezone(gmtOffset,latitude,longitude);
	}
		
	 
	{
		int    gmtOffset =  -8;     // PST
		double longitude =  -122.865947;
		double latitude  =  42.235389;

		eeMgr.set_timezone(gmtOffset,latitude,longitude);
	}
	
	// Setup Time Sync Manager
	tsMgr.init();
	
	// Setup temp sensor
	tempSensor.setup();
	
	// setup PowerMgr
	powerMgr.begin(pin_ACpower);

	// Setup Buttons
	buttons.begin(BUTTON_GREEN,IC2_ADDR_BUTTON_GREEN);
	buttons.begin(BUTTON_RED, IC2_ADDR_BUTTON_RED);
//	buttons.self_test();

	door.begin(IC2_ADDR_RELAY, pin_enclosureDoor,pin_coopDoorClosed );
//	door.self_test();
	
	// self test complete
	display.clear();

	setupComplete = true;
}
 

void loop() {
 
	time_status tstatus = tsMgr.timeSyncStatus();

// MARK: setup the network
	if(!networkisSetup){
		pref_bits_entry prefs;
		
		// Setup the networking
		eeMgr.get_prefBits(&prefs);
		
		if(prefs.wifiOnStartup) {
			wifiMgr.setAPConnection(true);
		}
		
		networkisSetup = true;
	}
	
	// MARK: Setup the command line
	if(!cmdLineisSetup){
		cmdLineMgr.begin(&Serial, &Serial);
		cmdLineisSetup = true;
	}

// MARK: load the schedule
	if( tstatus == TIME_SYNCED && !eventsAreSetup) {
/*
	 set event 0 reload 00:00
	 set event 1 l1-on cr -10
	 set event 2 open cr
	 set event 3 l1-off sr +10
	 set event 4 close cs
	 set event 5 rl4-on 10:59
	 set event 6 rl4-off 11:00
*/
		
		scdMgr.loadSchedule();
		eventsAreSetup = true;
	}
	
	
	// MARK: reconcile any Events that might have occured before we run
	if(tstatus == TIME_SYNCED && eventsAreSetup && !eventsAreReconciled){
		reconcileEvents(&Serial);
		eventsAreReconciled = true;
	}
	
	
	// MARK: Process any new events
	if(eventsAreReconciled) {
		SavedEvent event = SE_NONE;
		uint32_t   when = 0;
		
		while(scdMgr.getNextEvent(&event, &when))
		{
			processEvent(event, when, &Serial);
		}
	}
 
// MARK: DO periodic updates
	wifiMgr.loop();
	buttons.loop();
	cmdLineMgr.loop();
	if(telnetPortMgr._running){
		telnetPortMgr.loop();
	}
	if(restPortMgr._running){
		restPortMgr.loop();
	}

  	stateMgr.loop();
	powerMgr.loop();
	
	display.updateDisplay();

	// resync if needed
	tsMgr.syncClockIfNeeded();

	// "Pet" the dog.
	if(setupComplete) am_hal_wdt_restart();

	delay(40);
}
 


