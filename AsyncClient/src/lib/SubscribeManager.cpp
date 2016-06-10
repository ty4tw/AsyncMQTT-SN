/*
 * SubscribeManager.cpp
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
    #include "MqttsnClient.h"
	#include "SubscribeManager.h"
	#include "GwProxy.h"
	#include "Timer.h"
#else
  	#include <MqttsnClientApp.h>
    #include <MqttsnClient.h>
	#include <SubscribeManager.h>
	#include <GwProxy.h>
	#include <Timer.h>
#endif
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace tomyAsyncClient;

extern void setUint16(uint8_t* pos, uint16_t val);
extern uint16_t getUint16(const uint8_t* pos);
extern MqttsnClient* theClient;
/*========================================
       Class SubscribeManager
 =======================================*/
SubscribeManager::SubscribeManager(){
    _first  = 0;
}

SubscribeManager::~SubscribeManager(){
	SubElement* elm = _first;
	SubElement* sav = 0;
	while (elm){
		sav = elm->next;
		if (elm != 0){
			free(elm);
		}
		elm = sav;
	}
}


SubElement* SubscribeManager::add(uint8_t msgType, const char* topicName, uint16_t topicId, uint8_t topicType, uint8_t qos, TopicCallback callback, uint16_t msgId){
	SubElement* last = _first;
	SubElement* prev = _first;
	SubElement* elm = (SubElement*) calloc(1,sizeof(SubElement));

	if (elm == 0){
		return 0;
	}
	if (last == 0){
		_first = elm;
	}
    elm->msgType = msgType;
    elm->callback = callback;
	elm->topicName = topicName;
    elm->topicId = topicId;
    elm->topicType = topicType;
    if (qos == 0){
        elm->qos = MQTTSN_FLAG_QOS_0;
/*
    }else if ( qos == 1){
        elm->qos = MQTTSN_FLAG_QOS_1;
    }else if (qos == 2){
        elm->qos = MQTTSN_FLAG_QOS_2;
    }
*/
    }else{
        elm->qos = MQTTSN_FLAG_QOS_1;
    }
	elm->msgId = msgId;
	elm->retryCount = MQTTSN_RETRY_COUNT;
	elm->sendUTC = 0;

	while(last){
		prev = last;
		if (prev->next != 0){
			last = prev->next;
		}else{
			prev->next = elm;
			elm->prev = prev;
			elm->next = 0;
			last = 0;
		}
	}
	return elm;
}

void SubscribeManager::remove(SubElement* elm){
    if (elm){
    	if (elm->prev == 0){
    		_first = elm->next;
    		if (elm->next != 0){
    			elm->next->prev = 0;
    		}
    		free(elm);
    	}else{
    		elm->prev->next = elm->next;
    		free(elm);
    	}
    }
}

bool SubscribeManager::isDone(void){
	return _first == 0;
}

void SubscribeManager::send(SubElement* elm){
	uint8_t msg[MQTTSN_MAX_MSG_LENGTH + 1];
    if (elm->topicType == MQTTSN_TOPIC_TYPE_NORMAL){
        msg[0] = 5 + strlen(elm->topicName);
        strcpy((char*)msg + 5, elm->topicName);
    }else{
        msg[0] = 7;
        setUint16(msg + 5, elm->topicId);
    }
    msg[1] = elm->msgType;
    msg[2] = elm->qos | elm->topicType;
    if ((elm->retryCount < MQTTSN_RETRY_COUNT) && elm->msgType == MQTTSN_TYPE_SUBSCRIBE){
        msg[2] = msg[2] | MQTTSN_FLAG_DUP;
    }
    setUint16(msg + 3, elm->msgId);
    
	theClient->getGwProxy()->writeMsg(msg);
	theClient->getGwProxy()->resetPingReqTimer();
    elm->sendUTC = Timer::getUnixTime();
    elm->retryCount--;
}


SubElement* SubscribeManager::getElement(uint16_t msgId){
	SubElement* elm = _first;
	while(elm){
		if (elm->msgId == msgId){
			return elm;
		}else{
			elm = elm->next;
		}
	}
	return 0;
}

SubElement* SubscribeManager::getElement(const char* topicName){
	SubElement* elm = _first;
	while(elm){
		if (strcmp(elm->topicName, topicName) == 0){
			return elm;
		}else{
			elm = elm->next;
		}
	}
	return 0;
}

SubElement* SubscribeManager::getElement(uint16_t topicId, uint8_t topicType){
	SubElement* elm = _first;
	while(elm){
		if (elm->topicId == topicId && elm->topicType == topicType){
			return elm;
		}else{
			elm = elm->next;
		}
	}
	return 0;
}

void SubscribeManager::subscribe(const char* topicName, TopicCallback onPublish, uint8_t qos){
	SubElement* elm = add(MQTTSN_TYPE_SUBSCRIBE, topicName, 0, MQTTSN_TOPIC_TYPE_NORMAL, qos, onPublish, theClient->getGwProxy()->getNextMsgId());
	send(elm);
}

void SubscribeManager::subscribe(uint16_t topicId, TopicCallback onPublish, uint8_t qos, uint8_t topicType){
	SubElement* elm = add(MQTTSN_TYPE_SUBSCRIBE, 0, topicId, topicType, qos, onPublish, theClient->getGwProxy()->getNextMsgId());
	send(elm);
}

void SubscribeManager::unsubscribe(const char* topicName){
	SubElement* elm = add(MQTTSN_TYPE_UNSUBSCRIBE, topicName, 0, MQTTSN_TOPIC_TYPE_NORMAL, 0, 0, theClient->getGwProxy()->getNextMsgId());
	send(elm);
}

void SubscribeManager::unsubscribe(uint16_t topicId, uint8_t topicType){
	SubElement* elm = add(MQTTSN_TYPE_UNSUBSCRIBE, 0, topicId, topicType, 0, 0, theClient->getGwProxy()->getNextMsgId());
	send(elm);
}


void  SubscribeManager::checkTimeout(void){
	SubElement* elm = _first;
	SubElement* sav;
	while (elm){
		if ( elm->sendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
			if (elm->retryCount >= 0){
				send(elm);
			}else{
				if (elm->next){
					sav = elm->prev;
					remove(elm);
					if(sav){
						elm = sav;
					}else{
						break;
					}
				}else{
					remove(elm);
					break;
				}
			}
		}
		elm = elm->next;
	}
}


void SubscribeManager::responce(const uint8_t* msg){
    if ( msg[0] == MQTTSN_TYPE_SUBACK){
        uint16_t topicId = getUint16(msg + 2);
        uint16_t msgId = getUint16(msg + 4);
        uint8_t rc = msg[6];
        if (rc == 0){
            TopicTable* tt = theClient->getGwProxy()->getTopicTable();
            SubElement* elm = getElement(msgId);
            if (elm){
            	tt->add((char*)elm->topicName, topicId, elm->topicType, elm->callback);
            }
        }
        remove(getElement(msgId));
    }else{
        remove(getElement(getUint16(msg + 1)));
    }
}
