#include "common.h"

#include "Arduino.h"
#include "avr/pgmspace.h"
#include "SPI.h"

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "display.h"

const char string1[] PROGMEM = "Starting...";
const char string2[] PROGMEM = " Latitude: ";
const char string3[] PROGMEM = "Longitude: ";
const char string4[] PROGMEM = " Altitude: ";
const char string5[] PROGMEM = "  Speed: ";
const char string6[] PROGMEM = "Bearing: ";
const char string7[] PROGMEM = "Satellites: ";
const char string8[] PROGMEM = "Fixquality: ";
const char string9[] PROGMEM = "      HDOP: ";
const char string10[] PROGMEM = "Failed to open file";
const char string11[] PROGMEM = "Logging...";
const char string12[] PROGMEM = "#Records: ";
const char string13[] PROGMEM = "Distance: ";

const char *const string_table[] PROGMEM = {
    string1, string2, string3, string4, string5, string6, string7, string8, string9, string10, string11
    , string12, string13
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
    , ID_DISTANCE
};

enum DisplayScreen {
    DISP_BEGIN = 0,
    DISP_LATLON = 1,
    DISP_SPEEDBER = 2,
    DISP_SATFIX = 3,
    DISP_TIME = 4,
    DISP_TRACK = 5,
    DISP_END = 6
};

int aprintf(Adafruit_SSD1306& display, char *str, ...) {
    int i, j, count = 0;

    va_list argv;
    va_start(argv, str);
    for(i = 0, j = 0; str[i] != '\0'; i++) {
        if (str[i] == '%') {
            count++;

            display.Print::write(reinterpret_cast<const uint8_t*>(str+j), (size_t)(i-j));

            switch (str[++i]) {
                case 'S': 
                    {
                        char buf[32];
                        strcpy_P(buf, (char *)pgm_read_word(&string_table[va_arg(argv, int)]));
                        display.print(buf);
                    }
                    break;
                case 'd': display.print(va_arg(argv, int));
                          break;
                case 'l': display.print(va_arg(argv, long));
                          break;
                case 'f': display.print(va_arg(argv, double));
                          break;
                case 'c': display.print((char) va_arg(argv, int));
                          break;
                case 's': display.print(va_arg(argv, char *));
                          break;
                case '%': display.print("%");
                          break;
                default:;
            };

            j = i+1;
        }
    };
    va_end(argv);

    if(i > j) {
        display.Print::write(reinterpret_cast<const uint8_t*>(str+j), (size_t)(i-j));
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
, latitude(0.0)
, longitude(0.0)
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
  displayString(ID_STARTING);
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

void GPSDisplay::runningLogging()
{
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayString(ID_RUN_LOGGING);
    m_display.display();
}

void GPSDisplay::failedToOpenLogfile()
{
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayString(ID_LOGFILE_FAILED);
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
    }

    m_display.display();
}

void GPSDisplay::displayLatLon()
{
    if (m_currentDisplayScreen != DISP_LATLON)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayPair(ID_LATITUDE, latitude);
    displayPair(ID_LONGITUDE, longitude);
    displayPair(ID_ALTITUDE, altitude);
}

void GPSDisplay::displaySpeedBer()
{
    if (m_currentDisplayScreen != DISP_SPEEDBER)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayPair(ID_SPEED, speed*1.15078);
    displayPair(ID_BEARING, bearing);
}

void GPSDisplay::displaySatFix()
{
    if (m_currentDisplayScreen != DISP_SATFIX)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayPair(ID_SATELLITES, satellites);
    displayPair(ID_FIXQUALITY, fixquality);
    displayPair(ID_HDOP, HDOP);
}

void GPSDisplay::displayTrack()
{
    if (m_currentDisplayScreen != DISP_TRACK)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    displayPair(ID_NUMRECS, numrecs);
    displayPair(ID_DISTANCE, distance);
}

void GPSDisplay::displayTime()
{
    if (m_currentDisplayScreen != DISP_TIME)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    m_display.print(month);
    m_display.print("/");
    m_display.print(day);
    m_display.print("/");
    m_display.println(year);
    m_display.print(hour);
    m_display.print(":");
    m_display.print(minute);
    m_display.print(":");
    m_display.print(seconds);
    m_display.print(".");
    m_display.print(milliseconds);
    m_display.print(" UTC");
}

void GPSDisplay::displayString(int id)
{
    char buf[32];
    strcpy_P(buf, (char *)pgm_read_word(&string_table[id]));
    m_display.print(buf);
}

void GPSDisplay::displayPair(int id, float& f)
{
    char buf[32];
    strcpy_P(buf, (char *)pgm_read_word(&string_table[id]));
    m_display.print(buf);
    m_display.println(f);
}

void GPSDisplay::displayPair(int id, int n)
{
    char buf[32];
    strcpy_P(buf, (char *)pgm_read_word(&string_table[id]));
    m_display.print(buf);
    m_display.println(n);
}
