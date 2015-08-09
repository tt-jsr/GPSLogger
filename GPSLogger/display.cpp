#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "display.h"

const char string1[] PROGMEM = "Starting...";
const char string2[] PROGMEM = " Lat: %d%c %f'\n";
const char string3[] PROGMEM = "Long: %d%c %f'\n";
const char string4[] PROGMEM = " Alt: %f";
const char string5[] PROGMEM = "  Speed: %f\n";
const char string6[] PROGMEM = "Bearing: %f";
const char string7[] PROGMEM = "Satellites: %d\n";
const char string8[] PROGMEM = "Fixquality: %d\n";
const char string9[] PROGMEM = "      HDOP: %f";
const char string10[] PROGMEM = "Failed to open file";
const char string11[] PROGMEM = "Logging...";
const char string12[] PROGMEM = "#Records: %d\n";
const char string13[] PROGMEM = "Distance: %f ft\n";
const char string14[] PROGMEM = "Distance: %f mi\n";
const char string15[] PROGMEM = "Run Time: %dh %dm %ds";
const char string16[] PROGMEM = "%2d/%2d/20%d\n%2d:%2d:%2d UTC";
const char string17[] PROGMEM = "GPSLogger\n  by Jeff Richards\n";
const char string18[] PROGMEM = "Version 1.1";

const char *const string_table[] PROGMEM = {
    string1, string2, string3, string4, string5, string6, string7, string8, string9, string10, string11
    , string12, string13, string14, string15, string16, string17, string18
};

enum STRING_ID {
    ID_STARTING
    , ID_LATITUDE
    , ID_LONGITUDE
    , ID_ALTITUDE
    , ID_SPEED
    , ID_BEARING
    , ID_SATELLITES
    , ID_FIXQUALITY
    , ID_HDOP
    , ID_LOGFILE_FAILED
    , ID_RUN_LOGGING
    , ID_NUMRECS
    , ID_DISTANCE_FT
    , ID_DISTANCE_MI
    , ID_RUNTIME
    , ID_DATETIME
    , ID_ABOUT
    , ID_VERSION
};

void padding(Print& dest, byte pad, long n)
{
    if (pad == 0)
        return;
    while (n)
    {
        n /= 10;
        --pad;
    }
    if (pad <= 0)
        return;
    while (pad--)
        dest.print("0");
}

int aprintf(Print& dest, int ID, ...) {
    int i, j, count = 0;
    char buf[32];
    char *str = buf;
    if (ID >= 0 && ID < 100)
    {
        strncpy_P(buf, (char *)pgm_read_word(&string_table[ID]), sizeof(buf));
        buf[sizeof(buf)-1] = '\0';
    }
    else
    {
        str = (char *)ID;
    }
    va_list argv;
    va_start(argv, str);
    for(i = 0, j = 0; str[i] != '\0'; i++) {
        byte pad = 0;
        if (str[i] == '%') {
            count++;
            dest.Print::write(reinterpret_cast<const uint8_t*>(str+j), (size_t)(i-j));
            if (str[i+1] >= '0' && str[i+1] <= '9')
            {
                ++i;
                pad = str[i] - '0';
                ++count;
            }

            switch (str[++i]) {
                case 'S': 
                    {
                        char buf[32];
                        strcpy_P(buf, (char *)pgm_read_word(&string_table[va_arg(argv, int)]));
                        dest.print(buf);
                    }
                    break;
                case 'd': 
                      {
                          int n =  va_arg(argv, int);
                          padding(dest, pad, n);
                          dest.print(n);
                      }
                          break;
                case 'l':
                      {
                          long n =  va_arg(argv, long);
                          padding(dest, pad, n);
                          dest.print(n);
                      }
                          break;
                case 'f': dest.print(va_arg(argv, double));
                          break;
                case 'c': dest.print((char) va_arg(argv, int));
                          break;
                case 's': dest.print(va_arg(argv, char *));
                          break;
                case '%': dest.print("%");
                          break;
                default:;
            };

            j = i+1;
        }
    };
    va_end(argv);

    if(i > j) {
        dest.Print::write(reinterpret_cast<const uint8_t*>(str+j), (size_t)(i-j));
    }

    return count;
}
/*************************************************/
StatusLights::StatusLights()
: m_leds(0xff)
{
    SetFixStatus(false);
    SetLoggingStatus(LOG_DISABLED);
}

void StatusLights::SetFixStatus(bool b)
{
  bitSet(m_leds, 4); // green
  bitSet(m_leds, 3);
  if (b)
    bitClear(m_leds, 4);
  else
    bitClear(m_leds, 3);
  SendStatusLights();
}

void StatusLights::SetLoggingStatus(int loggingStatus)
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
  SendStatusLights();
}

void StatusLights::SendStatusLights()
{
  digitalWrite(PIN_SR_LATCH, LOW);
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(m_leds);
  SPI.endTransaction();
  digitalWrite(PIN_SR_LATCH, HIGH);
}

/***************************************************/

GPSDisplay::GPSDisplay()
: m_display(OLED_DC, OLED_RESET, OLED_CS)
, latDegrees(0)
, latMinutes(0.0)
, lonDegrees(0)
, lonMinutes(0.0)
, altitude(0.0)
, bearing(0.0)
, speed(0.0)
, HDOP(0.0)
, fixquality(0)
, satellites(0)
, lat_ns(0)
, lon_ew(0)
, hour(0)
, minute(0)
, seconds(0)
, milliseconds(0)
, day(0)
, month(0)
, year(0)
, numrecs(0)
, distance(0.0)
, m_currentDisplayScreen(DISP_LATLON)
{
}

void GPSDisplay::setup()
{
  m_display.begin();
  m_display.clearDisplay();
  m_display.setTextSize(1);
  m_display.setTextColor(WHITE);
}

void GPSDisplay::splashScreen()
{
  m_display.clearDisplay();
  aprintf(m_display, ID_STARTING);
  refresh();
}

void GPSDisplay::nextScreen()
{
    m_currentDisplayScreen += 1;
    if (m_currentDisplayScreen == DISP_END)
        m_currentDisplayScreen = 1;
    refresh();
}

void GPSDisplay::prevScreen()
{
    --m_currentDisplayScreen;
    if (m_currentDisplayScreen == DISP_BEGIN)
        m_currentDisplayScreen = DISP_END-1;
    refresh();
}

void GPSDisplay::firstScreen()
{
    m_currentDisplayScreen = 1;
    refresh();
}

void GPSDisplay::lastScreen()
{
    m_currentDisplayScreen = DISP_END-1;
    refresh();
}

void GPSDisplay::setScreen(int screen)
{
    m_currentDisplayScreen = screen;
    refresh();
}

void GPSDisplay::runningLogging()
{
    m_display.clearDisplay();
    m_display.setCursor(32, 12);
    aprintf(m_display, ID_RUN_LOGGING);
    m_display.display();
}

void GPSDisplay::failedToOpenLogfile()
{
    m_display.clearDisplay();
    m_display.setTextCursor(0, 0);
    aprintf(m_display, ID_LOGFILE_FAILED);
    m_display.display();
}

void GPSDisplay::refresh()
{
    switch(m_currentDisplayScreen)
    {
        case DISP_LATLON:
            displayLatLon();
            break;
        case DISP_SPEEDBER:
            displaySpeedBer();
            break;
        case DISP_SATFIX:
            displaySatFix();
            break;
        case DISP_TIME:
            displayTime();
            break;
        case DISP_TRACK:
            displayTrack();
            break;
        case DISP_ABOUT:
            displayAbout();
            break;
    }

    m_display.display();
}

void GPSDisplay::displayLatLon()
{
    char buf[32];
    if (m_currentDisplayScreen != DISP_LATLON)
        return;
    m_display.clearDisplay();
    // 248 is the degree symbol
    aprintf(m_display, ID_LATITUDE, latDegrees, 248, latMinutes);
    aprintf(m_display, ID_LONGITUDE, lonDegrees, 248, lonMinutes);
    aprintf(m_display, ID_ALTITUDE, altitude*3.2808);
}

void GPSDisplay::displaySpeedBer()
{
    if (m_currentDisplayScreen != DISP_SPEEDBER)
        return;
    m_display.clearDisplay();
    aprintf(m_display, ID_SPEED, speed*1.15078);
    aprintf(m_display, ID_BEARING, bearing);
}

void GPSDisplay::displaySatFix()
{
    if (m_currentDisplayScreen != DISP_SATFIX)
        return;
    m_display.clearDisplay();
    aprintf(m_display, ID_SATELLITES, satellites);
    aprintf(m_display, ID_FIXQUALITY, fixquality);
    aprintf(m_display, ID_HDOP, HDOP);
}

void GPSDisplay::displayTrack()
{
    if (m_currentDisplayScreen != DISP_TRACK)
        return;
    m_display.clearDisplay();
    aprintf(m_display, ID_NUMRECS, numrecs);
    if (distance > 1610.0)
    {
        aprintf(m_display, ID_DISTANCE_MI, distance*.00062137);
    }
    else
    {
        aprintf(m_display, ID_DISTANCE_FT, distance*3.28984);
    }
    unsigned long now = millis();
    int hours = millis()/3600000;
    now -= hours * 3600000;
    int min = now/60000;
    now -= min*60000;
    int sec = now/1000;
    aprintf(m_display, ID_RUNTIME, hours, min, sec);
}

void GPSDisplay::displayTime()
{
    if (m_currentDisplayScreen != DISP_TIME)
        return;
    m_display.clearDisplay();
    aprintf(m_display, ID_DATETIME, month, day, year, hour, minute, seconds);
}

void GPSDisplay::displayAbout()
{
    if (m_currentDisplayScreen != DISP_ABOUT)
        return;
    m_display.clearDisplay();
    aprintf(m_display, ID_ABOUT);
    aprintf(m_display, ID_VERSION);
}


