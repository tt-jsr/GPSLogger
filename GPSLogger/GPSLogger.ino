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
GPS gps(&mySerial);
StatusLights statusLights;

enum {
    LOOP_DEFAULT
    , LOOP_LOGGING
    ,LOOP_DISPLAY
};

byte runLoop = LOOP_DISPLAY;

File *pLogfile = NULL;
GPSDisplay *pDisplay = NULL;
bool openFailed = false;

#define LOG_INTERVAL  10

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
        if (runLoop == LOOP_DISPLAY)
        {
            if (digitalRead(PIN_NAV_UP) == LOW)
                if (pDisplay) pDisplay->firstScreen();
            if (digitalRead(PIN_NAV_DOWN) == LOW)
                if (pDisplay) pDisplay->lastScreen();
            if (digitalRead(PIN_NAV_LEFT) == LOW)
                if (pDisplay) pDisplay->prevScreen();
            if (digitalRead(PIN_NAV_RIGHT) == LOW)
                if (pDisplay) pDisplay->nextScreen();
        }
        if (digitalRead(PIN_BUT_DISP) == LOW)
        {
            switch(runLoop)
            {
            case LOOP_DEFAULT:
                runLoop = LOOP_DISPLAY;
                break;
            case LOOP_DISPLAY:
                runLoop = LOOP_LOGGING;
                if (pDisplay) pDisplay->runningLogging();
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

int logcounter = 0;
int linecounter = 0;

#define MAXLINELEN 85
byte lineidx = 0;
char nmeaLine[MAXLINELEN];

void field_callback_logging(char c)
{
    //Serial.print(c);
    if (pLogfile == NULL)
        return;
    nmeaLine[lineidx++] = c;
    if (lineidx == MAXLINELEN)
        --lineidx;
    if (c == '\n')
    {
        ++logcounter;
        nmeaLine[lineidx] = 0;
        statusLights.SetLoggingStatus(LOG_ENABLED);
        statusLights.SendStatusLights();
        pLogfile->print(nmeaLine);
        //pLogfile->flush();
        lineidx = 0;
        return;
    }
    statusLights.SetLoggingStatus(LOG_WRITING);
    statusLights.SendStatusLights();
}

void field_callback(char c)
{
    if (c == '\n')
    {
        ++linecounter;
    }
    if ((linecounter % LOG_INTERVAL) == 0)
        field_callback_logging(c);
}

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

    SD.begin(SD_CS);
    statusLights.SetLoggingStatus(LOG_DISABLED);
    statusLights.SetFixStatus(false);
    statusLights.SendStatusLights();
    gps.register_field_callback(field_callback);
}

#define LOG_INTERVAL 10
void loggingLoop()
{
    File logfile = SD.open("datalog.nmea", FILE_WRITE);
    if (!logfile)
    {
        //Serial.println("File opened error");
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
    GPSDisplay display;
    display.setup();
    display.splashScreen();
    pDisplay = &display;
    gps.setdisplay(&display);
    if (openFailed)
    {
        display.failedToOpenLogfile();
        delay(1000);
        openFailed = false;
    }
    while (runLoop == LOOP_DISPLAY)
    {
        if (gps.isDataAvailable())
        {
            gps.clearDataAvailable();
            statusLights.SetFixStatus(gps.fix);
            statusLights.SendStatusLights();
            display.refresh();
        }
        buttonThread.execute();
        if (serialEventRun) serialEventRun();
    }
    pDisplay = NULL;
    gps.setdisplay(NULL);
}

void loop() {
    buttonThread.execute();
    if (runLoop == LOOP_LOGGING)
        loggingLoop();
    if (runLoop == LOOP_DISPLAY)
        displayLoop();
}

