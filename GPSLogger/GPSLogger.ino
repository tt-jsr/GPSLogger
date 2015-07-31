#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#ifdef USE_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#include "SD.h"
#include "Thread.h"
#include "ThreadController.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "display.h"
#include "GPS.h"

#ifdef USE_SOFTWARE_SERIAL
SoftwareSerial mySerial(8, 7);
#else
HardwareSerial mySerial = Serial;
#endif
GPSDisplay display;
GPS gps(&mySerial, &display);

// This uses the very nice AuduinoThread library
// https://github.com/ivanseidel/ArduinoThread

class ButtonThread: public Thread
{
public:
    void setup()
    {
      pinMode(PIN_NAV_LEFT, INPUT);
      digitalWrite(PIN_NAV_LEFT, HIGH);
      pinMode(PIN_NAV_RIGHT, INPUT);
      digitalWrite(PIN_NAV_RIGHT, HIGH);
      pinMode(PIN_OE_ENABLE, OUTPUT);
      digitalWrite(PIN_OE_ENABLE, LOW);
      pinMode(PIN_SR_LATCH, OUTPUT);
      pinMode(PIN_NAV_UP, INPUT);
      digitalWrite(PIN_NAV_UP, HIGH);
      pinMode(PIN_NAV_DOWN, INPUT);
      digitalWrite(PIN_NAV_DOWN, HIGH);
      pinMode(PIN_NAV_SEL, INPUT);
      digitalWrite(PIN_NAV_SEL, HIGH);
      pinMode(PIN_BDISP, INPUT_PULLUP);

      setInterval(500);
    }
    void execute()
    {
        if (shouldRun())
           run();
    }
private:
	void run()
    {
        if (digitalRead(PIN_NAV_LEFT) == LOW)
            display.prevScreen();
        if (digitalRead(PIN_NAV_RIGHT) == LOW)
            display.nextScreen();
		runned();
	}
};

ButtonThread buttonThread;
File logfile;

void setup() {
  //Serial.begin(9600);

  buttonThread.setup();

  /**** GPS SETUP *****/
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  gps.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data

  // Set the update rate
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // Every 1 secs update rate

  // Request updates on antenna status, comment out to keep quiet
  //gps.sendCommand(PGCMD_ANTENNA);

  //SD.begin(SD_CS);
  //logfile = SD.open("datalog.txt", FILE_WRITE);
  
  display.setup();
  display.splashScreen();
}

#define LOG_INTERVAL 10
byte counter = 0;
void loop() {
    char c = 0;
    if (gps.isDataAvailable(c))
    {
        gps.clearDataAvailable();
        display.refresh();
    }
    if (c)
    {
        if (c == '$')
            ++counter;
        if (counter == LOG_INTERVAL)
        {
            //logfile.write(c);
            if (c == '\n')
                counter = 0;
        }
    }
    buttonThread.execute();
}

