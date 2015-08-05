#include "OneTime.h"

OneTime::OneTime(void (*callback)(void))
{
	_onRun = callback;
	_cached_next_run = ~0;
    _enabled = false;
};

void OneTime::runned(){
	// Cache next run
	_cached_next_run = ~0;
    _enabled = false;
}

void OneTime::setAt(unsigned long _at){
	// Save interval
	_cached_next_run = _at;
    _enabled = true;
}

void OneTime::secondsFromNow(unsigned long _fromNow){
	setAt((_fromNow*1000) + millis());
}

void OneTime::milliSecondsFromNow(unsigned long _fromNow){
	setAt(_fromNow + millis());
}

bool OneTime::shouldRun(unsigned long time){
	// If the "sign" bit is set the signed difference would be negative
	bool time_remaining = (time - _cached_next_run) & 0x80000000;

	// Exceeded the time limit? Then should run...
	return !time_remaining && _enabled;
}

void OneTime::run(){
	if(_onRun != NULL)
		_onRun();
}
