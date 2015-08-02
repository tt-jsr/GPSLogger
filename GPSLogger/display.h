#ifndef DISPLAY_H_
#define DISPLAY_H_

enum LoggingStatus
{
  LOG_DISABLED = 0,
  LOG_ENABLED = 1,
  LOG_WRITING = 2
};

class StatusLights
{
public:
    StatusLights();
    void SetLoggingStatus(int loggingStatus);
    void SetFixStatus(bool);
    void SendStatusLights();
private:
    byte m_leds;
};

class GPSDisplay
{
public:
    GPSDisplay();
    void setup();
    void splashScreen();
    void firstScreen();
    void lastScreen();
    void nextScreen();
    void prevScreen();
    void refresh();
    void runningLogging();
    void failedToOpenLogfile();
public:
    float latitude;
    float longitude;
    float altitude;
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
    int numrecs;
    float distance;
private:
    void displayLatLon();
    void displaySpeedBer();
    void displaySatFix();
    void displayMessage();
    void displayTime();
    void displayTrack();
    void displayString(int stringid);
private:
    int m_currentDisplayScreen;
    Adafruit_SSD1306 m_display;
};

#endif
