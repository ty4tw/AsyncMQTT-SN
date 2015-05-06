/*
 * Topics.h
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 tomoaki@tomy-tech.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *  Created on: 2015/05/01
 *    Modified:
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 0.0.0
 */

#ifndef TOPICS_H_
#define TOPICS_H_

#include "ProcessFramework.h"
#include "Messages.h"
#include <stdlib.h>

#define MQTTSN_TOPIC_MULTI_WILDCARD   '#'
#define MQTTSN_TOPIC_SINGLE_WILDCARD  '+'

#define MQTTSN_TOPICID_NORMAL 256
#define MQTTSN_TOPICID_PREDEFINED_TIME   0x0001
#define MQTTSN_TOPIC_PREDEFINED_TIME     ("$GW/01")

#define MAX_TOPIC_COUNT   50        // Number of Topic Par ClientNode
namespace tomyAsyncGateway{
/*=====================================
        Class Topic
======================================*/
typedef int (*TopicCallback)(void);

class Topic {
	friend class Topics;
public:
    Topic();
    Topic(string topic);
    ~Topic();

    //int     execCallback(void);
    string*  getTopicName(void);
    uint16_t getTopicId(void);
    int      hasWildCard(unsigned int* pos);
    bool     isMatch(string* topicName);
private:
    uint16_t _topicId;
    uint8_t  _topicType;
    string   _topicName;
    TopicCallback  _callback;
    Topic*   _next;
};

/*=====================================
        Class Topics
 ======================================*/
class Topics {
public:
      Topics();
      ~Topics();
      Topic*    add(string* topic, uint16_t id = 0, uint8_t type = MQTTSN_TOPIC_TYPE_NORMAL, TopicCallback callback = 0);
      uint16_t  getTopicId(string* topic);
      uint16_t  getNextTopicId();
      Topic*    getTopic(uint16_t topicId, uint8_t topicType);
      Topic*    getTopic(string* topic);
      Topic*    match(string* topic);
private:
    uint16_t _nextTopicId;
    Topic*   _first;

};
}
#endif /* TOPICS_H_ */
