/*
 * TaskManager.h
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
 *  Created on: 2015/04/15
 *    Modified: 
 *      Author: tomoaki
 *     Version: 0.0.1
 */

#ifndef TASKMANAGER_H_
#define TASKMANAGER_H_

#ifdef ARDUINO
    #include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#ifdef ARDUINO
  #include <Timer.h>

  #if defined( NW_DEBUG) || defined(MQTTSN_DEBUG)
        #include <SoftwareSerial.h>
        extern SoftwareSerial debug;
  #endif

#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include <stdio.h>
  #include <string.h>
#endif /* LINUX */

using namespace std;

namespace tomyAsyncClient {
struct TaskList{
    int (*callback)(void);
	uint32_t interval;
    uint32_t prevTime;;
};


#define TASK_DONE    0
#define TASK_RUN     1
#define TASK_SUSPEND 2

/*========================================
       Class TaskManager
 =======================================*/
class TaskManager{
public:
    TaskManager();
    ~TaskManager();
    void add(TaskList* task);
    void run(void);
private:
    TaskList* _task;

};

} /* tomyAsyncClient */
#endif /* TASKMANAGER_H_ */

