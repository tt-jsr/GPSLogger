#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#include <SoftwareSerial.h>
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "display.h"

#include "JSD.h"
#include "Thread.h"
#include "OneTime.h"
#include "ThreadController.h"
#include "GPS.h"

enum {
    LOOP_DEFAULT
    , LOOP_LOGGING
    ,LOOP_DISPLAY
};

// This uses the very nice AuduinoThread library
// https://github.com/ivanseidel/ArduinoThread

class ButtonThread: public Thread
{
public:
    void setup();
    void execute();
private:
	void run();
};

class TurnOffWriteStatus : public OneTime
{
public:
    void setup();
    void execute();
private:
    void run();
};


class RMCThread: public Thread
{
public:
    void setup();
    void execute(char *p);
private:
	void run(char *p);
};

class GGAThread: public Thread
{
public:
    void setup();
    void execute(char *p);
private:
	void run(char *p);
};


SoftwareSerial mySerial(8, 7);
GPS gps(&mySerial);
StatusLights statusLights;
byte runLoop = LOOP_DISPLAY;
File *pLogfile = NULL;
bool openFailed = false;
GPSDisplay *pDisplay = NULL;
int numrecs = 0;
int LOG_INTERVAL = 5000;
bool fix = false;

ButtonThread buttonThread;
RMCThread rmcThread;
GGAThread ggaThread;
TurnOffWriteStatus turnOffWriteStatus;

void sentence_callback(char *sentence)
{
    if (strlen(sentence) > 6)
    {
        if ((strncmp(sentence, "$GPRMC", 6) == 0))
        {
            rmcThread.execute(sentence);
        }
        if ((strncmp(sentence, "$GPGGA", 6) == 0))
        {
            ggaThread.execute(sentence);
        }
    }
}

void setup() {
    //Serial.begin(9600);
    buttonThread.setup();
    rmcThread.setup();
    ggaThread.setup();
    turnOffWriteStatus.setup();

    gps.setup(4800);
    statusLights.SetLoggingStatus(LOG_DISABLED);
    statusLights.SetFixStatus(false);
    pLogfile = NULL;
    gps.register_sentence_callback(sentence_callback);
    delay(20);
}

bool commonLoopStuff()
{
    bool rtn = false;
    if (gps.isDataAvailable())
    {
        gps.clearDataAvailable();
        gps.calculateDistance();
        rtn = true;
        if (gps.fix==true && fix==false)
        {
            statusLights.SetFixStatus(gps.fix);
            fix = gps.fix;
            if (pLogfile) pLogfile->println("Acquired fix");
            if (pDisplay) pDisplay->setScreen(DISP_LATLON);
        }
        if (gps.fix == false && fix == true)
        {
            statusLights.SetFixStatus(gps.fix);
            fix = gps.fix;
            if (pLogfile) pLogfile->println("Lost fix");
            if (pDisplay) pDisplay->setScreen(DISP_SATFIX);
        }
    }
    buttonThread.execute();
    turnOffWriteStatus.execute();
    if (serialEventRun) serialEventRun();
    return rtn;
}

void loggingLoop()
{

    cache_t cache;
    SdVolume::SetCache(cache);
    SD.begin(SD_CS);
    File logfile;
    logfile = SD.open("DATALOG.TXT", O_WRITE | O_CREAT | O_APPEND);
    if (!logfile)
    {
        statusLights.SetLoggingStatus(LOG_DISABLED);
        openFailed = true;
        runLoop = LOOP_DISPLAY;
        return;
    }
    openFailed = false;
    pLogfile = &logfile;
    statusLights.SetLoggingStatus(LOG_ENABLED);
    while (runLoop == LOOP_LOGGING)
    {
        commonLoopStuff();
    }
    logfile.close();
    pLogfile = NULL;
    statusLights.SetLoggingStatus(LOG_DISABLED);
}

void displayLoop()
{
    GPSDisplay display;
    display.setup();
    delay(10);
    //display.splashScreen();
    pDisplay = &display;
    gps.setdisplay(&display);
    pLogfile = NULL;
    if (openFailed)
    {
        display.failedToOpenLogfile();
        delay(1000);
        openFailed = false;
    }
    if (gps.fix == false)
    {
        display.setScreen(DISP_SATFIX);
    }
    display.numrecs = numrecs;
    while (runLoop == LOOP_DISPLAY)
    {
        if (commonLoopStuff())
        {
            display.refresh();
        }
    }
    pDisplay = NULL;
    gps.setdisplay(NULL);
}

void loop() {
    buttonThread.execute();
    if (runLoop == LOOP_LOGGING)
    {
        loggingLoop();
    }
    if (runLoop == LOOP_DISPLAY)
    {
        displayLoop();
    }
}
/****************************************************************/
void ButtonThread::setup()
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
void ButtonThread::execute()
{
    if (shouldRun())
       run();
}

void ButtonThread::run()
{
    if (runLoop == LOOP_DISPLAY)
    {
        if (digitalRead(PIN_NAV_UP) == LOW)
            if (pDisplay) pDisplay->setScreen(DISP_TRACK);
        if (digitalRead(PIN_NAV_DOWN) == LOW)
            if (pDisplay) pDisplay->setScreen(DISP_LATLON);
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

/**********************************************************/
void TurnOffWriteStatus::setup() 
{
}

void TurnOffWriteStatus::execute()
{
    if (shouldRun())
        run();
}

void TurnOffWriteStatus::run()
{
    statusLights.SetLoggingStatus(LOG_ENABLED);
    runned();
}

/**********************************************************/

void RMCThread::setup()
{
  setInterval(LOG_INTERVAL);
}

void RMCThread::execute(char *p)
{
    if (shouldRun())
       run(p);
}

void RMCThread::run(char *sentence)
{
    if (pLogfile == NULL)
        return;
    statusLights.SetLoggingStatus(LOG_WRITING);
    pLogfile->write(sentence);
    turnOffWriteStatus.milliSecondsFromNow(500);
    ++numrecs;
    runned();
}

/**********************************************************/

void GGAThread::setup()
{
  setInterval(LOG_INTERVAL);
}

void GGAThread::execute(char *p)
{
    if (shouldRun())
       run(p);
}

void GGAThread::run(char *sentence)
{
    if (pLogfile == NULL)
        return;
    statusLights.SetLoggingStatus(LOG_WRITING);
    pLogfile->write(sentence);
    turnOffWriteStatus.milliSecondsFromNow(500);
    ++numrecs;
    runned();
}
