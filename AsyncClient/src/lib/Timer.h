/*
 * Timer.h
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

#ifndef TIMER_H_
#define TIMER_H_

#if defined(ARDUINO)
	#include <MqttsnClientApp.h>
    #if ARDUINO >= 100
        #include "Arduino.h"
        #include <inttypes.h>
    #else
        #if ARDUINO < 100
            #include "WProgram.h"
            #include <inttypes.h>
        #endif
    #endif
#else
	#include "MqttsnClientApp.h"
#endif /* ARDUINO */


#ifdef LINUX
    #include <sys/time.h>
#endif

namespace tomyAsyncClient {


#ifdef ARDUINO
/*============================================
       Timer for Arduino
 ============================================*/
class Timer{
public:
    Timer();
    ~Timer(){};
    void start(uint32_t msec = 0);
    bool isTimeUp(uint32_t msec);
    bool isTimeUp(void);
    void stop();
    static uint32_t getUnixTime();
    static void setUnixTime(uint32_t utc);
	static void setStopTimeDuration(uint32_t msec);
    static void initialize();
    static void changeUTC();

private:
    uint32_t _startTime;
    uint32_t _currentTime;
    uint32_t _millis;
	uint32_t _timeupUnixTime;
	static uint32_t _unixTime;
	static uint32_t _epochTime;
	static uint32_t _timerStopTimeAccum;
	static bool _utcFlag;

};


#endif


#ifdef LINUX
/*============================================
                Timer
 ============================================*/
class Timer{
public:
    Timer();
    ~Timer();
    void start(uint32_t msec = 0);
    bool isTimeUp(uint32_t msec);
    bool isTimeUp(void);
    void stop(void);
    void changeUTC(void){};
    static uint32_t getUnixTime();
    static void setUnixTime(uint32_t utc){};
private:
    struct timeval _startTime;
    uint32_t _millis;
};

#endif
}


#endif /* TIMER_H_ */
