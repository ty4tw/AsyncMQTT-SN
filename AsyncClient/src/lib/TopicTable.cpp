/*
 * TopicTable.cpp
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
        #include <TopicTable.h>
#else
        #include "MqttsnClientApp.h"
        #include "TopicTable.h"
#endif

#if defined(ARDUINO) && ARDUINO >= 100
        #include "Arduino.h"
        #include <inttypes.h>
#endif

#if defined(ARDUINO) && ARDUINO < 100
        #include "WProgram.h"
        #include <inttypes.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace tomyAsyncClient;
/*=====================================
        Class Topic
 ======================================*/
Topic::Topic(){
    _topicStr = 0;
    _callback = 0;
    _topicId = 0;
    _topicType = MQTTSN_TOPIC_TYPE_NORMAL;
    _next = 0;
    _malocFlg = 0;
}

Topic::~Topic(){
	if (_malocFlg){
		free(_topicStr);
	}
}

TopicCallback Topic::getCallback(void){
	return _callback;
}

int Topic::execCallback(Payload* payload){
    if(_callback != 0){
        return _callback(payload);
    }
    return 0;
}


uint8_t Topic::hasWildCard(uint8_t* pos){
	*pos = strlen(_topicStr) - 1;
    if (*(_topicStr + *pos) == '#'){
        return MQTTSN_TOPIC_MULTI_WILDCARD;
    }else{
    	for(uint8_t p = 0; p < strlen(_topicStr); p++){
    		if (*(_topicStr + p) == '+'){
    			*pos = p;
    			return MQTTSN_TOPIC_SINGLE_WILDCARD;
    		}
    	}
    }
    return 0;
}

bool Topic::isMatch(char* topic){
    uint8_t pos;

	if ( strlen(topic) < strlen(_topicStr)){
		return false;
	}

	uint8_t wc = hasWildCard(&pos);

	if (wc == MQTTSN_TOPIC_SINGLE_WILDCARD){
		if ( strncmp(_topicStr, topic, pos - 1) == 0){
			if (*(_topicStr + pos + 1) == '/'){
				for(uint8_t p = pos; p < strlen(topic); p++){
					if (*(topic + p) == '/'){
						if (strcmp(_topicStr + pos + 1, topic + p ) == 0){
							return true;
						}
					}
				}
			}else{
				for(uint8_t p = pos + 1;p < strlen(topic); p++){
					if (*(topic + p) == '/'){
						return false;
					}
				}
			}
			return true;
		}
	}else if (wc == MQTTSN_TOPIC_MULTI_WILDCARD){
		if (strncmp(_topicStr, topic, pos) == 0){
			return true;
		}
	}else if (strcmp(_topicStr, topic) == 0){
		return true;
	}
	return false;
}


/*=====================================
        Class TopicTable
 ======================================*/
TopicTable::TopicTable(){
	_first = 0;
}

TopicTable::~TopicTable(){
	clearTopic();
}


Topic* TopicTable::getTopic(char* topic){
	Topic* p = _first;
	while(p){
		if (p->_topicStr != 0 && strcmp(p->_topicStr, topic) == 0){
			return p;
		}
		p = p->_next;
	}
	return 0;
}

Topic* TopicTable::getTopic(uint16_t topicId, uint8_t topicType){
	Topic* p = _first;
	while(p){
		if (p->_topicId == topicId && p->_topicType == topicType){
			return p;
		}
		p = p->_next;
	}
	return 0;
}

uint16_t TopicTable::getTopicId(char* topic){
	Topic* p = getTopic(topic);
	if (p){
		return p->_topicId;
	}
	return 0;
}


char* TopicTable::getTopicName(Topic* topic){
	return topic->_topicStr;
}


void TopicTable::setTopicId(char* topic, uint16_t id, uint8_t type){
    Topic* tp = getTopic(topic);
    if (tp){
        tp->_topicId = id;
    }else{
    	add(topic, id, type, 0);
    }
}


bool TopicTable::setCallback(char* topic, TopicCallback callback){
	Topic* p = getTopic(topic);
	if (p){
		p->_callback = callback;
		return true;
	}
	return false;
}


bool TopicTable::setCallback(uint16_t topicId, uint8_t topicType, TopicCallback callback){
	Topic* p = getTopic(topicId, topicType);
	if (p){
		p->_callback = callback;
		return true;
	}
	return false;
}


int TopicTable::execCallback(uint16_t  topicId, Payload* payload, uint8_t topicType){
	Topic* p = getTopic(topicId, topicType);
	if (p){;
		return p->execCallback(payload);
	}
	return 0;
}


Topic* TopicTable::add(char* topicName, uint16_t id, uint8_t type, TopicCallback callback, uint8_t alocFlg){
    Topic* elm;
    if (topicName){
	    elm = getTopic(topicName);
    }else{
        elm = getTopic(id, type);
    }
    
	if (elm == 0){
		Topic* tp = _first;
		elm = new Topic();
		if(elm == 0){
			return 0;
		}
		elm->_topicStr = topicName;
		elm->_topicId = id;
        elm->_topicType = type;
		elm->_callback = callback;
		elm->_malocFlg = alocFlg;

		if (tp == 0){
			_first = elm;
		}
		while(tp){
			if(tp->_next == 0){
				tp->_next = elm;
				break;
			}else{
				tp = tp->_next;
			}
		}
	}else{
		elm->_callback = callback;
	}
	return elm;
}


Topic* TopicTable::match(char* topicName){
	Topic* elm = _first;
	while(elm){
		if (elm->isMatch(topicName)){
			return elm;
		}
		elm = elm->_next;
	}
	return 0;
}


void TopicTable::clearTopic(void){
	Topic* p = _first;
	while(p){
		_first = p->_next;
		delete p;
		p = _first;
	}
}
