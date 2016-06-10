/*
 * PubSubManager.h
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

#ifndef PUBLISHMANAGER_H_
#define PUBLISHMANAGER_H_

#ifdef ARDUINO
    #include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#ifdef ARDUINO
  #include <Timer.h>
  #include <Payload.h>
  #include <TopicTable.h>

  #if defined( NW_DEBUG) || defined(MQTTSN_DEBUG)
        #include <SoftwareSerial.h>
        extern SoftwareSerial debug;
  #endif

#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include "Payload.h"
  #include "TopicTable.h"
#endif /* LINUX */

using namespace std;

namespace tomyAsyncClient {

#define TOPICID_IS_SUSPEND  0
#define TOPICID_IS_READY    1
#define WAIT_PUBACK         2
#define WAIT_PUBREC         3
#define WAIT_PUBREL         4
#define WAIT_PUBCOMP        5

#define SAVE_TASK_INDEX     1
#define NEG_TASK_INDEX      0


typedef struct PubElement{
    uint16_t  msgId;
    uint16_t  topicId;
    const char* topicName;
    Payload*  payload;
    uint32_t  sendUTC;
    int       (*callback)(void);
    int       retryCount;
    int       taskIndex;
    PubElement* prev;
    PubElement* next;
    uint8_t   flag;
    uint8_t   status;  // 0:SUSPEND, 1:READY
} PubElement;

/*========================================
       Class PublishManager
 =======================================*/
class PublishManager{
public:
	PublishManager();
    ~PublishManager();
    void publish(const char* topicName, Payload* payload, uint8_t qos, bool retain);
    void publish(uint16_t topicId, Payload* payload, uint8_t qos);
    void responce(const uint8_t* msg, uint16_t msglen);
    void published(uint8_t* msg, uint16_t msglen);
    void checkTimeout(void);
    void sendSuspend(const char* topicName, uint16_t topicId, uint8_t topicType);
    bool isDone(void);
    bool isMaxFlight(void);
private:
    PubElement* getElement(uint16_t msgId);
    PubElement* getElement(const char* topicName);
    PubElement* add(const char* topicName, uint16_t topicId, Payload* payload, uint8_t qos, uint8_t retain, uint16_t msgId);
	void remove(PubElement* elm);
	void sendPublish(PubElement* elm);
    void sendPubAck(uint16_t topicId, uint16_t msgId, uint8_t rc);
    void sendPubRel(PubElement* elm);
    void delElement(PubElement* elm);
	PubElement* _first;
	uint8_t     _elmCnt;
	uint8_t     _publishedFlg;
};
 
} /* tomyAsyncClient */
#endif /* PUBLISHMANAGER_H_ */
