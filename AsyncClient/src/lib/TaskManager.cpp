/*
 * TaskManager.cpp
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

#ifdef ARDUINO
    #include <MqttsnClientApp.h>
#else
    #include "MqttsnClientApp.h"
#endif

#ifdef ARDUINO
  #include <Timer.h>
  #include <TaskManager.h>
  #include <MqttsnClient.h>
#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include "TaskManager.h"
  #include "MqttsnClient.h"
#endif /* LINUX */
#include <stdio.h>
#include <string.h>

using namespace std;
using namespace tomyAsyncClient;
extern MqttsnClient* theClient;
/*=====================================
           TaskManager
 ======================================*/
TaskManager::TaskManager(void){
	_task = 0;
	_index = 0;
}

TaskManager::~TaskManager(void){
    
}

void TaskManager::add(TaskList* task){
    _task = task;
}

void TaskManager::run(void){
	while (true){
		theClient->getGwProxy()->getMessage();
		for (_index = 0; _task[_index].callback > 0; _index++){
			if ((_task[_index].prevTime + _task[_index].interval < Timer::getUnixTime()) &&
				 _task[_index].count == 0){
				_task[_index].prevTime = Timer::getUnixTime();
				(_task[_index].callback)();
			}
		}

		do{
			theClient->getGwProxy()->getMessage();
		}while(theClient->getPublishManager()->isMaxFlight() ||
			   !theClient->getSubscribeManager()->isDone() ||
			   !theClient->getRegisterManager()->isDone());

		if (theClient->getPublishManager()->isDone()){
			break;
		}
	}
}

uint8_t TaskManager::getIndex(void){
	return _index;
}

void TaskManager::done(uint8_t index){
	if (_task[index].count > 0){
		_task[index].count--;
	}
}

void TaskManager::suspend(uint8_t index){
	_task[index].count++;
}
