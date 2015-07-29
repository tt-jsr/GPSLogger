#include "Arduino.h"
//#include "avr/pgmspace.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "SPI.h"
#include "common.h"
#include "display.h"

/*
char starting[] PROGMEM = "Starting...";
char latitude[] PROGMEM = " Latitude: ";
char longitude[] PROGMEM = "Longitude: ";
char altitude[] PROGMEM = " Altitude: ";
char speed[] PROGMEM = "  Speed: ";
char bearing[] PROGMEM = "Bearing: ";
char satellites[] PROGMEM = "Satellites: ";
char fixquality[] PROGMEM = "Fixquality: ";
char hdop[] PROGMEM = "      HDOP: ";
*/

enum DisplayScreen {
    DISP_BEGIN = 0,
    DISP_LATLON = 1,
    DISP_SPEEDBER = 2,
    DISP_SATFIX = 3,
    DISP_END = 4
};

GPSDisplay::GPSDisplay()
: m_latitude(0.0)
, m_longitude(0.0)
, m_altitude(0.0)
, m_bearing(0.0)
, m_speed(0.0)
, m_HDOP(0.0)
, m_fixquality(0)
, m_satellites(0)
, m_currentDisplayScreen(DISP_LATLON)
, m_leds(0xff)
, m_display(OLED_DC, OLED_RESET, OLED_CS)
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
  m_display.setTextSize(2);
  m_display.print("Starting...");
  refresh();
  m_display.setTextSize(1);
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

void GPSDisplay::setPosition(float lat, float lng, float altitude)
{
    m_latitude = lat;
    m_longitude = lng;
    m_altitude = altitude;
}

void GPSDisplay::setSpeedBearing(float speed, float angle)
{
    m_speed = speed;
    m_bearing = angle;
}

void GPSDisplay::setSatellites(byte fixquality, byte satellites, float hdop)
{
    m_fixquality = fixquality;
    m_satellites = satellites;
    m_HDOP = hdop;
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
    }

    m_display.display();
    SendStatusLights();
}

void GPSDisplay::displayLatLon()
{
    if (m_currentDisplayScreen != DISP_LATLON)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    m_display.print(" Latitude: ");
    m_display.println(m_latitude);
    m_display.print("Longitude: ");
    m_display.println(m_longitude);
    m_display.print(" Altitude: ");
    m_display.println(m_altitude);
}

void GPSDisplay::displaySpeedBer()
{
    if (m_currentDisplayScreen != DISP_SPEEDBER)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    m_display.print("  Speed: ");
    m_display.println(m_speed);
    m_display.print("Bearing: ");
    m_display.println(m_bearing);
}

void GPSDisplay::displaySatFix()
{
    if (m_currentDisplayScreen != DISP_SATFIX)
        return;
    m_display.clearDisplay();
    m_display.setCursor(0, 0);
    m_display.print("Satellites: ");
    m_display.println(m_satellites);
    m_display.print("Fixquality: ");
    m_display.println(m_fixquality);
    m_display.print("      HDOP: ");
    m_display.println(m_HDOP);
}

void GPSDisplay::SetFixStatus(bool b)
{
  bitSet(m_leds, 4); // green
  bitSet(m_leds, 3);
  if (b)
    bitClear(m_leds, 4);
  else
    bitClear(m_leds, 3);
}

void GPSDisplay::SetLoggingStatus(int loggingStatus)
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

void GPSDisplay::SendStatusLights()
{
  digitalWrite(PIN_SR_LATCH, LOW);
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(m_leds);
  SPI.endTransaction();
  digitalWrite(PIN_SR_LATCH, HIGH);
}
