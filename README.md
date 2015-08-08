# GPSLogger
This is an Arduino project to provide GPS logging capability to an SD card. I use this in conjunction with GPSBable
to convert NMEA sentences to gpx, and lightroom to geolocate my photos.

Becuase the SD card statically allocated its buffers and uses 720 bytes when linked in, and the Adafruit OLED display
driver also statically allocates its frame buffer and uses 610 bytes, I did not have enough ram to run both.

I modified the SD library (renamed as JSD) and modified the display driver to allow me to place these on the stack and have
either the display or the logger running.

GPSlogger.dsn is a TinyCad schematic of the circuit.

