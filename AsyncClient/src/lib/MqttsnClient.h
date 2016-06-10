/*
 * MqttsnClient.h
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

#ifndef MQTTSNCLIENT_H_
#define MQTTSNCLIENT_H_

#ifdef ARDUINO
	#include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#include <stdio.h>
#include <string.h>

using namespace std;
#ifdef ARDUINO
  #include <Timer.h>
  #include <TaskManager.h>
  #include <PublishManager.h>
  #include <SubscribeManager.h>
  #include <GwProxy.h>
  #include <Payload.h>
#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include "TaskManager.h"
  #include "PublishManager.h"
  #include "SubscribeManager.h"
  #include "GwProxy.h"
  #include "Payload.h"
#endif /* LINUX */

using namespace std;
namespace tomyAsyncClient {
struct OnPublishList{
	const char* topic;
	int (*pubCallback)(Payload*);
	uint8_t qos;
};

#define GETUTC() Timer::getUnixTime()
int setUTC(Payload*);
/*========================================
       Class MqttsnClient
 =======================================*/
class MqttsnClient{
public:
    MqttsnClient();
    ~MqttsnClient();
    void onConnect(void);
    void publish(const char* topicName, Payload* payload, uint8_t qos, bool retain = false);
    void subscribe(const char* topicName, TopicCallback onPublish, uint8_t qos);
    void subscribe(uint16_t topicId, TopicCallback onPublish, uint8_t qos, uint8_t topicType);
    void unsubscribe(const char* topicName);
    void disconnect(uint16_t sleepInSecs);
    void initialize(APP_CONFIG config);
    void run(void);
    int  sleep(void);
    void registerInt0Callback(void (*callback)());
    void addTask(void);
    void setSleepMode(bool mode);
    GwProxy*          getGwProxy(void);
    PublishManager*   getPublishManager(void);
    SubscribeManager* getSubscribeManager(void);
    RegisterManager*  getRegisterManager(void);
    TaskManager*      getTaskManager(void);
    TopicTable*       getTopicTable(void);
    void              indicator(bool onoff);
private:
    TaskManager      _taskMgr;
    PublishManager   _pubMgr;
    SubscribeManager _subMgr;
    GwProxy          _gwProxy;
    bool             _sleepMode;
    void            (*_intCallback)(void);
};


} /* tomyAsyncClient */
#endif /* MQTTSNCLIENT_H_ */
