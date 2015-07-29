#ifndef DISPLAY_H_
#define DISPLAY_H_

enum LoggingStatus
{
  LOG_DISABLED = 0,
  LOG_ENABLED = 1,
  LOG_WRITING = 2
};

class GPSDisplay
{
public:
    GPSDisplay();
    void setup();
    void splashScreen();
    void nextScreen();
    void prevScreen();
    void setPosition(float lat, float lng, float altitude);
    void setSpeedBearing(float speed, float angle);
    void setSatellites(byte fixquality, byte satellites, float hdop);
    void SetFixStatus(bool b);
    void SetLoggingStatus(int loggingStatus);
    void refresh();
private:
    void displayLatLon();
    void displaySpeedBer();
    void displaySatFix();
    void SendStatusLights();
    void displayString(int stringid);
private:
    float m_latitude;
    float m_longitude;
    float m_altitude;
    float m_bearing;
    float m_speed;
    float m_HDOP;
    byte m_fixquality;
    byte m_satellites;
    int m_currentDisplayScreen;
    byte m_leds;
    Adafruit_SSD1306 m_display;
};

#endif
