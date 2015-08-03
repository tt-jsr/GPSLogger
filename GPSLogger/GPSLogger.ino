#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#include <SoftwareSerial.h>

#include "JSD.h"
#include "Thread.h"
#include "ThreadController.h"
#include "GPS.h"

SoftwareSerial mySerial(8, 7);
GPS gps(&mySerial);

enum LoggingStatus
{
  LOG_DISABLED = 0,
  LOG_ENABLED = 1,
  LOG_WRITING = 2
};

class StatusLights
{
public:
    StatusLights()
    : m_leds(0xff)
    {
        SetFixStatus(false);
        SetLoggingStatus(LOG_DISABLED);
    }
    void SetLoggingStatus(int loggingStatus)
    {
      bitSet(m_leds, 0);
      bitSet(m_leds, 1);
      bitSet(m_leds, 2);
      
      switch(loggingStatus)
      {
        case LOG_DISABLED:
          bitClear(m_leds, 0);
          break;
        case LOG_ENABLED:
          bitClear(m_leds, 1);
          break;
        case LOG_WRITING:
          bitClear(m_leds, 2);
          break;
      }
    }
    void SetFixStatus(bool b)
    {
      bitSet(m_leds, 4); // green
      bitSet(m_leds, 3);
      if (b)
        bitClear(m_leds, 4);
      else
        bitClear(m_leds, 3);
    }
    void SendStatusLights()
    {
      digitalWrite(PIN_SR_LATCH, LOW);
      SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
      SPI.transfer(m_leds);
      SPI.endTransaction();
      digitalWrite(PIN_SR_LATCH, HIGH);
    }
private:
    byte m_leds;
};

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
    //Serial.print(p);
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
            //Serial.print(p);
            pLogfile->write(p);
            resetStatusLight = millis() + 500;
            readyRMC = millis() + LOG_INTERVAL;
            ++numrecs;
        }
        if ((strncmp(p, "$GPGGA", 6) == 0) && readyGGA < millis())
        {
            statusLights.SetLoggingStatus(LOG_WRITING);
            statusLights.SendStatusLights();
            //Serial.print(p);
            pLogfile->write(p);
            resetStatusLight = millis() + 500;
            readyGGA = millis() + LOG_INTERVAL;
            ++numrecs;
        }
    }
}

void setup() {
    Serial.begin(9600);
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
    cache_t cache;
    SdVolume::SetCache(cache);
    logfile = SD.open("DATALOG.TXT", O_WRITE | O_CREAT | O_APPEND);
    if (!logfile)
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

