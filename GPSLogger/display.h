#ifndef DISPLAY_H_
#define DISPLAY_H_

enum LoggingStatus
{
  LOG_DISABLED = 0,
  LOG_ENABLED = 1,
  LOG_WRITING = 2
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

class StatusLights
{
public:
    StatusLights();
    void SetLoggingStatus(int loggingStatus);
    void SetFixStatus(bool);
private:
    void SendStatusLights();
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
    void setScreen(int);
    void refresh();
    void runningLogging();
    void failedToOpenLogfile();
public:
    int latDegrees;
    float latMinutes;
    int lonDegrees;
    float lonMinutes;
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
private:
    int m_currentDisplayScreen;
    Adafruit_SSD1306 m_display;
};

int aprintf(Print& dest, int ID, ...);
#endif
