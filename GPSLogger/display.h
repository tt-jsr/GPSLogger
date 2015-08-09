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
    DISP_ABOUT = 6,
    DISP_END = 7
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
    void displayAbout();
private:
    int m_currentDisplayScreen;
    Adafruit_SSD1306 m_display;
};

/*
 * Printf style output for arduino
 * Can print to any Print derived object.
 * ID: The format string, A string id string_table[]
 *     You can also pass in a char * format casted to an int.
 * %S A string in the string_table
 * %[n]d A integer, with 0 padded. Pad length can be 0-9
 * %[n]l A long, with 0 padded. Pad length can be 0-9
 * %f    A double
 * %s    A char * string
 * %c    A char
 *
 * Buffer is only 32 chars for string_table[] format strings and
 * %S string_table args
 */
int aprintf(Print& dest, int ID, ...);
#endif
