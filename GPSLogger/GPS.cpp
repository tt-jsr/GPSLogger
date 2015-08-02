#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#ifdef GPS_USES_SOFTWARE_SERIAL
#include <SoftwareSerial.h>
#endif
#ifndef USING_SERIAL
#include "SD.h"
#endif
#include "GPS.h"

#define MAXFIELDLENGTH 10
#define MAXLINELENGTH 85


char linebuf[MAXLINELENGTH];
uint8_t lineidx=0;

boolean ggaready = false;
boolean rmcready = false;
boolean inStandbyMode;

void GPS::read(void) 
{
    char c = 0;

    if (paused) 
        return;

    if(!pSerial->available()) 
        return;
    c = pSerial->read();
    linebuf[lineidx++] = c;
    if (lineidx >= MAXLINELENGTH)
    {
        linebuf[MAXLINELENGTH-3] = '\r';
        linebuf[MAXLINELENGTH-2] = '\n';
        linebuf[MAXLINELENGTH-1] = 0;
        lineidx = MAXLINELENGTH-1;
        c = '\n';
    }
    if (c == '\n')
    {
        linebuf[lineidx] = 0;
        lineidx = 0;
        if (fieldCallback)
            fieldCallback(linebuf);
    }
}

#ifdef GPS_USES_SOFTWARE_SERIAL
GPS::GPS(SoftwareSerial *ser)
#else
GPS::GPS(HardwareSerial *ser) 
#endif
{
  common_init();     // Set everything to common state, then...
  pSerial = ser; // ...override gpsSwSerial with value passed.
}

// Initialization code used by all constructor types
void GPS::common_init(void) {
  pSerial = NULL; // Set both to NULL, then override correct
  rmcready   = false;
  ggaready   = false;
  paused      = false;
  fieldidx     = 0;
  currentField = FIELD_UNKNOWN;
  currentSentence = SENTENCE_UNKNOWN;
  fieldCallback = NULL;
  fix = false;
}

void GPS::begin(uint16_t baud)
{
    pSerial->begin(baud);
  delay(10);
}

void GPS::sendCommand(const char *str) {
    pSerial->println(str);
}

void GPS::register_field_callback(FieldCallback cb)
{
    fieldCallback = cb;
}

boolean GPS::isDataAvailable() {
  read();
  if (ggaready && rmcready)
  {
     return true;
  }
  return false;
}

void GPS::clearDataAvailable(void) {
  rmcready = false;
  ggaready = false;
}

void GPS::pause(boolean p) {
  paused = p;
}

// read a Hex value and return the decimal equivalent
uint8_t GPS::parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F')
       return (c - 'A')+10;
    // if (c > 'F')
    return 0;
}

// Standby Mode Switches
boolean GPS::standby(void) {
  /*
  if (inStandbyMode) {
    return false;  // Returns false if already in standby mode, so that you do not wake it up by sending commands to GPS
  }
  else {
    inStandbyMode = true;
    sendCommand(PMTK_STANDBY);
    //return waitForSentence(PMTK_STANDBY_SUCCESS);  // don't seem to be fast enough to catch the message, or something else just is not working
    return true;
  }
  */
}

boolean GPS::wakeup(void) {
    /*
  if (inStandbyMode) {
   inStandbyMode = false;
    sendCommand("");  // send byte to wake it up
    return waitForSentence(PMTK_AWAKE);
  }
  else {
      return false;  // Returns false if not in standby mode, nothing to wakeup
  }
  */
}
