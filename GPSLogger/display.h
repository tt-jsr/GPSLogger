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
    void SetLoggingStatus(int loggingStatus);
    void refresh();
public:
    float latitude;
    float longitude;
    float altitude;
    bool fix;
    float bearing;
    float speed;
    float HDOP;
    byte fixquality;
    byte satellites;
    char lat_ns;
    char lon_ew;
    byte hour;
    byte minute;
    byte seconds;
    int milliseconds;
    byte day;
    byte month;
    int year;
    char message[32];
private:
    void displayLatLon();
    void displaySpeedBer();
    void displaySatFix();
    void displayMessage();
    void SendStatusLights();
    void SetFixStatus(bool);
    void displayString(int stringid);
private:
    int m_currentDisplayScreen;
    byte m_leds;
    Adafruit_SSD1306 m_display;
};

#endif
