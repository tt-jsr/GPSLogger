#include "Wire.h"
#include "SPI.h"
#include "SoftwareSerial.h"
#include "Adafruit_GPS.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "common.h"
#include "display.h"

SoftwareSerial mySerial(8, 7);
GPSDisplay display;
Adafruit_GPS GPS(&mySerial);

void setup() {
  Serial.begin(9600);
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

  /**** GPS SETUP *****/
  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ);   // Every 5 secs update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Request updates on antenna status, comment out to keep quiet
  //GPS.sendCommand(PGCMD_ANTENNA);

  display.setup();
  display.SetFixStatus(false);
  display.SetLoggingStatus(LOG_DISABLED);
  display.splashScreen();
}

#define BUTTON_HOLD 1000

void loop() {
    if (GPS.isDataAvailable())
    {
        GPS.clearDataAvailable();
        display.setPosition(GPS.latitudeDegrees, GPS.longitudeDegrees, GPS.altitude);
        display.setSpeedBearing(GPS.speed, GPS.angle);
        display.setSatellites(GPS.fixquality, GPS.satellites, GPS.HDOP);
        display.SetFixStatus(GPS.fix);

        display.refresh();
    }
    
    if ((millis()%500) == 0)
    {
        if (digitalRead(PIN_NAV_LEFT) == LOW)
            display.prevScreen();
        if (digitalRead(PIN_NAV_RIGHT) == LOW)
            display.nextScreen();
    }
}
