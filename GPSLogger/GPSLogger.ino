#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#ifdef GPS_USES_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#ifndef OUTPUT_SERIAL
#include "SD.h"
#endif
#include "Thread.h"
#include "ThreadController.h"
#include "GPS.h"

SoftwareSerial mySerial(8, 7);
GPS gps(&mySerial);
StatusLights statusLights;

enum {
    LOOP_DEFAULT
    , LOOP_LOGGING
    ,LOOP_DISPLAY
};

byte runLoop = LOOP_DISPLAY;

File *pLogfile = NULL;
bool openFailed = false;

#define LOG_INTERVAL  10000 // in milliseconds

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
        if (digitalRead(PIN_BUT_DISP) == LOW)
        {
            switch(runLoop)
            {
            case LOOP_DEFAULT:
                runLoop = LOOP_DISPLAY;
                break;
            case LOOP_DISPLAY:
                runLoop = LOOP_LOGGING;
                break;
            case LOOP_LOGGING:
                runLoop = LOOP_DISPLAY;
                break;
            }
        }
		runned();
	}
};

ButtonThread buttonThread;

int numrecs = 0;
unsigned long resetStatusLight = ~0;
unsigned long readyRMC = 0;
unsigned long readyGGA = 0;
void field_callback(char *p)
{
   //DEBUG(p);
    if (resetStatusLight < millis())
    {
        statusLights.SetLoggingStatus(LOG_ENABLED);
        statusLights.SendStatusLights();
        resetStatusLight = ~0;
    }
    if (pLogfile == NULL)
        return;

    if (strlen(p) > 6)
    {
        if ((strncmp(p, "$GPRMC", 6) == 0) && readyRMC < millis())
        {
            statusLights.SetLoggingStatus(LOG_WRITING);
            statusLights.SendStatusLights();
            pLogfile->write(p);
            resetStatusLight = millis() + 500;
            readyRMC = millis() + LOG_INTERVAL;
            ++numrecs;
        }
        if ((strncmp(p, "$GPGGA", 6) == 0) && readyGGA < millis())
        {
            statusLights.SetLoggingStatus(LOG_WRITING);
            statusLights.SendStatusLights();
            pLogfile->write(p);
            resetStatusLight = millis() + 500;
            readyGGA = millis() + LOG_INTERVAL;
            ++numrecs;
        }
    }
}

void setup() {
    buttonThread.setup();

    /**** GPS SETUP *****/
    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    gps.begin(4800);

    //gps.sendCommand(PMTK_SET_BAUD_4800);
    //gps.begin(4800);
    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    // uncomment this line to turn on only the "minimum recommended" data

    // Set the update rate
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // Every 1 secs update rate

    // Request updates on antenna status, comment out to keep quiet
    //gps.sendCommand(PGCMD_ANTENNA);

    SD.begin(SD_CS);
    statusLights.SetLoggingStatus(LOG_DISABLED);
    statusLights.SetFixStatus(false);
    statusLights.SendStatusLights();
    gps.register_field_callback(field_callback);
}

void loggingLoop()
{
    File logfile;
    bool r = SD.open("DATALOG.TXT", O_WRITE | O_CREAT | O_APPEND);
    if (!r)
    {
        statusLights.SetLoggingStatus(LOG_DISABLED);
        statusLights.SendStatusLights();
        openFailed = true;
        runLoop = LOOP_DISPLAY;
        return;
    }
    openFailed = false;
    pLogfile = &logfile;
    statusLights.SetLoggingStatus(LOG_ENABLED);
    statusLights.SendStatusLights();
    while (runLoop == LOOP_LOGGING)
    {
        if (gps.isDataAvailable())
        {
            statusLights.SetFixStatus(gps.fix);
            statusLights.SendStatusLights();
            gps.clearDataAvailable();
        }
        buttonThread.execute();
        if (serialEventRun) serialEventRun();
    }
    logfile.close();
    pLogfile = NULL;
    statusLights.SetLoggingStatus(LOG_DISABLED);
    statusLights.SendStatusLights();
}

void displayLoop()
{
    pLogfile = NULL;
    while (runLoop == LOOP_DISPLAY)
    {
        if (gps.isDataAvailable())
        {
            gps.clearDataAvailable();
            statusLights.SetFixStatus(gps.fix);
            statusLights.SendStatusLights();
        }
        buttonThread.execute();
        if (serialEventRun) serialEventRun();
    }
}

void loop() {
    buttonThread.execute();
    if (runLoop == LOOP_LOGGING)
        loggingLoop();
    if (runLoop == LOOP_DISPLAY)
        displayLoop();
}

