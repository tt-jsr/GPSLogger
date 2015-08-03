#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"
//#include "Adafruit_GFX.h"
//#include "Adafruit_SSD1306.h"
//#include "display.h"

#include <SoftwareSerial.h>
#include "SD.h"
#include "GPS.h"

#define MAXFIELDLENGTH 10
#define MAXLINELENGTH 85

char linebuf[MAXLINELENGTH];
uint8_t lineidx=0;

boolean ggaready = false;
boolean rmcready = false;
boolean inStandbyMode;

char field[MAXFIELDLENGTH];
uint8_t fieldidx=0;

enum {
    FIELD_UNKNOWN
    , FIELD_TIME
    , FIELD_VALIDITY
    , FIELD_LATITUDE
    , FIELD_LONGITUDE
    , FIELD_ALTITUDE
    , FIELD_BEARING
    , FIELD_SPEED
    , FIELD_FIXQUALITY
    , FIELD_SATELLITES
    , FIELD_HDOP
    , FIELD_LAT_NS
    , FIELD_LON_EW
    , FIELD_DATE
    , FIELD_ANTENNA_UNITS
    , FIELD_GEOID
    , FIELD_UNITS_GEOID
    , FIELD_DIFF_AGE
    , FIELD_DIFF_STATION
    , FIELD_MAGVAR
    , FIELD_MAGVAR_EW
    , FIELD_CHECKSUM
};

enum {
    SENTENCE_UNKNOWN
    , SENTENCE_GGA
    , SENTENCE_RMC
};

bool GPS::parseGGA(char *field)
{
   switch(currentField)
   {
       case FIELD_TIME:
           {
            //char *p = field;
            //float timef = atof(p);
            //uint32_t time = timef;
            //if (pDisplay)
            //{
                //pDisplay->hour = time / 10000;
                //pDisplay->minute = (time % 10000) / 100;
                //pDisplay->seconds = (time % 100);
//
                //pDisplay->milliseconds = fmod(timef, 1.0) * 1000;
            //}
            currentField = FIELD_LATITUDE;
           }
           break;
        case FIELD_LATITUDE:
           currentField = FIELD_LAT_NS;
           break;
        case FIELD_LAT_NS:
           currentField = FIELD_LONGITUDE;
           break;
        case FIELD_LONGITUDE:
           currentField = FIELD_LON_EW;
           break;
        case FIELD_LON_EW:
           currentField = FIELD_FIXQUALITY;
           break;
        case FIELD_FIXQUALITY:
           {
               //if (pDisplay) pDisplay->fixquality = atoi(field);
               currentField = FIELD_SATELLITES;
           }
           break;
        case FIELD_SATELLITES:
           {
               //if (pDisplay) pDisplay->satellites = atoi(field);
               currentField = FIELD_HDOP;
           }
           break;
        case FIELD_HDOP:
           {
               //if (pDisplay) pDisplay->HDOP = atof(field);
               currentField = FIELD_ALTITUDE;
           }
           break;
        case FIELD_ALTITUDE:
           {
               //if (pDisplay) pDisplay->altitude = atof(field);
               currentField = FIELD_ANTENNA_UNITS;
           }
           break;
        case FIELD_ANTENNA_UNITS:
           currentField = FIELD_GEOID;
           break;
        case FIELD_GEOID:
           currentField = FIELD_UNITS_GEOID;
           break;
        case FIELD_UNITS_GEOID:
           currentField = FIELD_DIFF_AGE;
           break;
        case FIELD_DIFF_AGE:
           currentField = FIELD_DIFF_STATION;
           break;
        case FIELD_DIFF_STATION:
           currentField = FIELD_CHECKSUM;
           break;
        case FIELD_CHECKSUM:
           break;
        default:
           //TODO: display error
           break;
   }
   return true;
}

bool GPS::parseRMC(char *field)
{
   switch(currentField)
   {
       case FIELD_TIME:
           {
            //char *p = field;
            //float timef = atof(p);
            //uint32_t time = timef;
            //if (pDisplay)
            //{
                //pDisplay->hour = time / 10000;
                //pDisplay->minute = (time % 10000) / 100;
                //pDisplay->seconds = (time % 100);
//
                //pDisplay->milliseconds = fmod(timef, 1.0) * 1000;
            //}
            currentField = FIELD_VALIDITY;
           }
           break;
       case FIELD_VALIDITY:
           {
            if (field[0] == 'A') 
            {
              fix = true;
            }
            else if (field[0] == 'V')
            {
              fix = false;
            }
            currentField = FIELD_LATITUDE;
           }
           break;
       case FIELD_LATITUDE:
           {
              //char *p = field;
              //char degreebuff[10];
              //strncpy(degreebuff, p, 2);
              //p += 2;
              //degreebuff[2] = '\0';
              //int32_t degree = atol(degreebuff) * 10000000;
              //strncpy(degreebuff, p, 2); // minutes
              //p += 3; // skip decimal point
              //strncpy(degreebuff + 2, p, 4);
              //degreebuff[6] = '\0';
              //long minutes = 50 * atol(degreebuff) / 3;
              //float tmp = degree / 100000 + minutes * 0.000006F;
              //if (pDisplay)
              //{
                  //pDisplay->latitude = (tmp-100*int(tmp/100))/60.0;
                  //pDisplay->latitude += int(tmp/100);
              //}
              currentField = FIELD_LAT_NS;
           }
           break;
       case FIELD_LAT_NS:
           {
              //if (field[0] == 'S') 
              //{
                  //if (pDisplay)
                  //{
                      //pDisplay->latitude *= -1.0;
                      //pDisplay->lat_ns = 'S';
                  //}
              //}
              //else if (field[0] == 'N') 
              //{
                  //if (pDisplay) pDisplay->lat_ns = 'N';
              //}
              //else if (field[0] == '\0') 
              //{
                  //if (pDisplay) pDisplay->lat_ns = 0;
              //}
              currentField = FIELD_LONGITUDE;
           }
           break;
       case FIELD_LONGITUDE:
           {
              //char degreebuff[10];
              //char *p = field;
              //strncpy(degreebuff, p, 3);
              //p += 3;
              //degreebuff[3] = '\0';
              //int32_t degree = atol(degreebuff) * 10000000;
              //strncpy(degreebuff, p, 2); // minutes
              //p += 3; // skip decimal point
              //strncpy(degreebuff + 2, p, 4);
              //degreebuff[6] = '\0';
              //long minutes = 50 * atol(degreebuff) / 3;
              //float tmp = degree / 100000 + minutes * 0.000006F;
              //if (pDisplay)
              //{
                  //pDisplay->longitude = (tmp-100*int(tmp/100))/60.0;
                  //pDisplay->longitude += int(tmp/100);
              //}
              currentField = FIELD_LON_EW;
           }
           break;
       case FIELD_LON_EW:
           {
              //if (field[0] == 'W') 
              //{
                  //if (pDisplay)
                  //{
                      //pDisplay->longitude *= -1.0;
                      ////pDisplay->lon_ew = 'W';
                  //}
              //}
              //else if (field[0] == 'E') 
              //{
                  //if (pDisplay) pDisplay->lon_ew = 'E';
              //}
              //else if (field[0] == '\0') 
              //{
                  //if (pDisplay) pDisplay->lon_ew = 0;
              //}
              currentField = FIELD_SPEED;
           }
           break;
        case FIELD_SPEED:
           {
               //if (pDisplay) pDisplay->speed = atof(field);
               currentField = FIELD_BEARING;
           }
           break;
        case FIELD_BEARING:
           {
               //if (pDisplay) pDisplay->bearing = atof(field);
               currentField = FIELD_DATE;
           }
           break;
        case FIELD_DATE:
           {
              //uint32_t fulldate = atof(field);
              //if (pDisplay)
              //{
                  //pDisplay->day = fulldate / 10000;
                  //pDisplay->month = (fulldate % 10000) / 100;
                  //pDisplay->year = (fulldate % 100);
              //}
              currentField = FIELD_MAGVAR;
           }
           break;
        case FIELD_MAGVAR:
           currentField = FIELD_MAGVAR_EW;
           break;
        case FIELD_MAGVAR_EW:
           currentField = FIELD_CHECKSUM;
           break;
        case FIELD_CHECKSUM:
           break;
        default:
           //TODO: display error
           break;
   }
   return true;
}
bool GPS::parse(char *field)
{
   if (strcmp(field, "$GPGGA") == 0)
   {
       currentSentence = SENTENCE_GGA;
       currentField = FIELD_TIME;

       return true;
   }
   else if (strcmp(field, "$GPRMC") == 0)
   {
       currentSentence = SENTENCE_RMC;
       currentField = FIELD_TIME;
       return true;
   }
   if (currentSentence == SENTENCE_GGA)
       return parseGGA(field);
   if (currentSentence == SENTENCE_RMC)
       return parseRMC(field);

   return false;
}

void GPS::endOfField()
{
    field[fieldidx] = 0;
    parse(field);
    fieldidx = 0;
}

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
        char *p = linebuf;
        while (*p)
        {
           switch (*p)
           {
           case '\r':
                break;
           case '\n':
                if (currentSentence == SENTENCE_RMC)
                {
                    rmcready = true;
                }
                if (currentSentence == SENTENCE_GGA)
                {
                    ggaready = true;
                }
                endOfField();
                break;
           case ',':
                endOfField();
                break;
           case '*':
                endOfField();
                break;
           default:
                field[fieldidx++] = *p;
                if (fieldidx >= MAXFIELDLENGTH)
                {
                    fieldidx = MAXFIELDLENGTH-1;
                }
                break;
            }
            ++p;
        }
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
