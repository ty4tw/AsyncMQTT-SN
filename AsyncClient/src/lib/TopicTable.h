/*
 * TopicTable.h
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

#ifndef TOPICTABLE_H_
#define TOPICTABLE_H_

#ifdef ARDUINO
	#include <MqttsnClientApp.h>
	#include <Payload.h>
#else
	#include "MqttsnClientApp.h"
	#include "Payload.h"
#endif

#include <stdio.h>

#define MQTTSN_TOPIC_MULTI_WILDCARD   1
#define MQTTSN_TOPIC_SINGLE_WILDCARD  2

namespace tomyAsyncClient {
/*=====================================
        Class Topic
 ======================================*/
typedef int (*TopicCallback)(Payload*);

class Topic {
	friend class TopicTable;
public:
    Topic();
    ~Topic();
    int      execCallback(Payload* msg);
    uint8_t  hasWildCard(uint8_t* pos);
    bool     isMatch(char* topic);
    TopicCallback getCallback(void);
private:
    uint16_t  _topicId;
    uint8_t   _topicType;
    char*     _topicStr;
    TopicCallback  _callback;
    uint8_t  _malocFlg;
    Topic*    _next;
};

/*=====================================
        Class TopicTable
 ======================================*/
class TopicTable {
public:
	TopicTable();
      ~TopicTable();
      uint16_t getTopicId(char* topic);
      char*    getTopicName(Topic* topic);
      Topic*   getTopic(char* topic);
      Topic*   getTopic(uint16_t topicId, uint8_t topicType = MQTTSN_TOPIC_TYPE_NORMAL);
      void     setTopicId(char* topic, uint16_t id, uint8_t topicType);
      bool     setCallback(char* topic, TopicCallback callback);
      bool     setCallback(uint16_t topicId, uint8_t type, TopicCallback callback);
      int      execCallback(uint16_t  topicId, Payload* payload, uint8_t topicType = MQTTSN_TOPIC_TYPE_NORMAL);
      Topic*   add(char* topic, uint16_t id = 0, uint8_t type = MQTTSN_TOPIC_TYPE_NORMAL, TopicCallback callback = 0, uint8_t alocFlg = 0);
      Topic*   match(char* topic);
      void     clearTopic(void);

private:
    Topic*  _first;

};

};

#endif /* TOPICTABLE_H_ */
