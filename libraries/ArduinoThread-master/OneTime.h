/*
 	OneTime.h - Run once at a certain time. Can be reset

	For instructions, go to https://github.com/ivanseidel/ArduinoThread

	Created by Ivan Seidel Gomes, March, 2013.
	Released into the public domain.
*/

#ifndef OneTime_h
#define OneTime_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <WProgram.h>
#endif

#include <inttypes.h>

class OneTime{
protected:
	// Scheduled run in Ms (MUST BE CACHED)	
	unsigned long _cached_next_run;

	// Callback for run() if not implemented
	void (*_onRun)(void);		

    bool _enabled;
public:

	OneTime(void (*callback)(void) = NULL);

	// Should run _fomNow milliseconds from now
	virtual void milliSecondsFromNow(unsigned long _fromNow);
	virtual void secondsFromNow(unsigned long _fromNow);

    // Should run at the given time.
	virtual void setAt(unsigned long _at);

	// Return if the Thread should be runned or not
	virtual bool shouldRun(unsigned long time);

	// Default is to check whether it should run "now"
	bool shouldRun() { return shouldRun(millis()); }

    void runned();

	// Runs Thread
	virtual void run();
};

#endif
