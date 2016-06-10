/*
 * Timer.cpp
 *                      The BSD License
 *
 *           Copyright (c) 2015, tomoaki@tomy-tech.com
 *                    All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ARDUINO
  	#include "MqttsnClientApp.h"
	#include "Timer.h"
	#include <stdlib.h>
	#include <string.h>
#else
  	#include <MqttsnClientApp.h>
	#include <Timer.h>
	#include <stdlib.h>
	#include <string.h>
#endif

using namespace std;
using namespace tomyAsyncClient;

/*=====================================
        Class Timer
 =====================================*/
#ifdef ARDUINO
/**
 *   for Arduino
 */
uint32_t Timer::_unixTime = 0;
uint32_t Timer::_epochTime = 0;
uint32_t Timer::_timerStopTimeAccum = 0;
bool    Timer::_utcFlag = false;


Timer::Timer(){
    stop();
}

void Timer::initialize(){
	_unixTime = 0;
	_epochTime = 0;
	_timerStopTimeAccum = 0;
	_utcFlag = false;
}

void Timer::start(uint32_t msec){
    _startTime = millis();
    _millis = msec;
    _currentTime = 0;
	_timeupUnixTime =  getUnixTime() + msec / 1000;
}

bool Timer::isTimeUp(){
    return isTimeUp(_millis);
}

bool Timer::isTimeUp(uint32_t msec){
	if(_utcFlag){
		_utcFlag = false;
		if(_timeupUnixTime > 1000000000 && _unixTime){
			return (getUnixTime() >= _timeupUnixTime);
		}else{
			return false;
		}
	}else{
		if ( _startTime){
			_currentTime = millis();
			if ( _currentTime < _startTime){
				return (0xffffffff - _startTime + _currentTime > msec);
			}else{
				return (_currentTime - _startTime > msec);
			}
		}else{
			return false;
		}
	}
}

void Timer::stop(){
    _startTime = 0;
    _millis = 0;
    _currentTime = 0;
    _timeupUnixTime = 0;
}

void Timer::setUnixTime(uint32_t utc){
    _epochTime = millis();
    _timerStopTimeAccum = 0;
    _unixTime = utc;
}

uint32_t Timer::getUnixTime(){
    uint32_t tm = _timerStopTimeAccum + millis();
    if (_epochTime > tm ){
        return _unixTime + (uint32_t)((0xffffffff - tm - _epochTime) / 1000);
    }else{
        return _unixTime + (uint32_t)((tm - _epochTime) / 1000);
    }
}

void Timer::setStopTimeDuration(uint32_t msec){
	_timerStopTimeAccum += msec;
}

void Timer::changeUTC(){
	_utcFlag = true;
}

#endif


#ifdef LINUX
/**
 *   for LINUX
 */
Timer::Timer(){
	_startTime.tv_sec = 0;
	_millis = 0;
}

Timer::~Timer(){

}

void Timer::start(uint32_t msec){
  gettimeofday(&_startTime, 0);
  _millis = msec;
}

bool Timer::isTimeUp(void){
  return isTimeUp(_millis);
}

bool Timer::isTimeUp(uint32_t msec){
    struct timeval curTime;
    uint32_t secs, usecs;
    if (_startTime.tv_sec == 0){
        return false;
    }else{
        gettimeofday(&curTime, 0);
        secs  = (curTime.tv_sec  - _startTime.tv_sec) * 1000;
        usecs = (curTime.tv_usec - _startTime.tv_usec) / 1000.0;
        return ((secs + usecs) > (uint32_t)msec);
    }
}

void Timer::stop(){
  _startTime.tv_sec = 0;
  _millis = 0;
}

uint32_t Timer::getUnixTime(){
	struct timeval curTime;
	gettimeofday(&curTime, 0);
	return curTime.tv_sec;
}
#endif
