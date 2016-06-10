/*
 * PublishManager.cpp
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
	#include "PublishManager.h"
	#include "GwProxy.h"
	#include "Timer.h"
#else
  	#include <MqttsnClientApp.h>
    #include <MqttsnClient.h>
	#include <PublishManager.h>
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
       Class PublishManager
 =======================================*/
const char* NULLCHAR = "";

PublishManager::PublishManager(){
    _first = 0;
    _elmCnt = 0;
    _publishedFlg = SAVE_TASK_INDEX;
}

PublishManager::~PublishManager(){
    PubElement* elm = _first;
	PubElement* sav = 0;
	while (elm){
		sav = elm->next;
		if (elm != 0){
			delElement(elm);
		}
		elm = sav;
	}
}

void PublishManager::publish(const char* topicName, Payload* payload, uint8_t qos, bool retain){
    PubElement* elm = add(topicName, 0, payload, qos, retain, theClient->getGwProxy()->getNextMsgId());
    if (elm->status == TOPICID_IS_READY){  
        sendPublish(elm);
    }else{
        theClient->getGwProxy()->registerTopic((char*)topicName, 0);
    }
}

void  PublishManager::publish(uint16_t topicId, Payload* payload, uint8_t qos){
	PubElement* elm = add(NULLCHAR, topicId, payload, qos, 0, theClient->getGwProxy()->getNextMsgId());
	sendPublish(elm);
}

void PublishManager::sendPublish(PubElement* elm){
	if (elm == 0){
		return;
	}

    theClient->getGwProxy()->connect();
    
    uint8_t msg[MQTTSN_MAX_MSG_LENGTH + 1];
    uint8_t org = 0;
    if (elm->payload->getLen() > 128){
        msg[0] = 0x01;
        setUint16(msg + 1, elm->payload->getLen() + 9);
        org = 3;
    }else{
        msg[0] = (uint8_t)elm->payload->getLen() + 7;
    }
    msg[org + 1] = MQTTSN_TYPE_PUBLISH;
    msg[org + 2] = elm->flag;
    if ((elm->retryCount < MQTTSN_RETRY_COUNT)){
        msg[org + 2] = msg[org + 2] | MQTTSN_FLAG_DUP;
    }
    if ((elm->flag & 0x03) == MQTTSN_TOPIC_TYPE_SHORT){
        memcpy( msg + org + 3, elm->topicName, 2);
    }else{
        setUint16(msg + org + 3, elm->topicId);
    }
    setUint16(msg + org + 5, elm->msgId);
    memcpy(msg + org + 7, elm->payload->getRowData(), elm->payload->getLen());

	theClient->getGwProxy()->writeMsg(msg);
	theClient->getGwProxy()->resetPingReqTimer();
    if (( elm->flag & 0x60) == MQTTSN_FLAG_QOS_0){
        remove(elm);  // PUBLISH Done
        return;
    }else if ((elm->flag & 0x60) == MQTTSN_FLAG_QOS_1){
        elm->status = WAIT_PUBACK;
    }else if ((elm->flag & 0x60) == MQTTSN_FLAG_QOS_2){
        elm->status = WAIT_PUBREC;
    }
    
    elm->sendUTC = Timer::getUnixTime();
    elm->retryCount--;
}


void PublishManager::sendSuspend(const char* topicName, uint16_t topicId, uint8_t topicType){
	PubElement* elm = _first;
	while (elm){
		if (strcmp(elm->topicName, topicName) == 0 && elm->status == TOPICID_IS_SUSPEND){
			elm->topicId = topicId;
			elm->flag |= topicType;
			elm->status = TOPICID_IS_READY;
			sendPublish(elm);
			elm = 0;
		}else{
			elm = elm->next;
		}
	}
}

void PublishManager::sendPubAck(uint16_t topicId, uint16_t msgId, uint8_t rc){
	uint8_t msg[7];
	msg[0] = 7;
	msg[1] = MQTTSN_TYPE_PUBACK;
	setUint16(msg + 2, topicId);
	setUint16(msg + 4, msgId);
	msg[6] = rc;
	theClient->getGwProxy()->writeMsg(msg);
}

void PublishManager::sendPubRel(PubElement* elm){
	uint8_t msg[4];
	msg[0] = 4;
	msg[1] = MQTTSN_TYPE_PUBREL;
	setUint16(msg + 2, elm->msgId);
	theClient->getGwProxy()->writeMsg(msg);
}


bool PublishManager::isDone(void){
	return (_first == 0);
}

bool PublishManager::isMaxFlight(void){
	return (_elmCnt > MAX_INFLIGHT_MSG / 2);
}

void PublishManager::responce(const uint8_t* msg, uint16_t msglen){
    if ( msg[0] == MQTTSN_TYPE_PUBACK){
        uint16_t msgId = getUint16(msg + 3);
        PubElement* elm = getElement(msgId);
        if (elm == 0){
        	return;
        }
        if (msg[5] == MQTTSN_RC_ACCEPTED ){            
            if (elm->status == WAIT_PUBACK){
                remove(elm); // PUBLISH Done
            }
        }else if (msg[5] == MQTTSN_RC_REJECTED_INVALID_TOPIC_ID){
            elm->status = TOPICID_IS_SUSPEND;
            elm->topicId = 0;
            elm->retryCount = MQTTSN_RETRY_COUNT;
            elm->sendUTC = 0;
            theClient->getGwProxy()->registerTopic((char*)elm->topicName, 0);
        }
    }else if (msg[0] == MQTTSN_TYPE_PUBREC){
        PubElement* elm = getElement(getUint16(msg + 1));
        if (elm == 0){
        	return;
        }
        if (elm->status == WAIT_PUBREC || elm->status == WAIT_PUBCOMP){
            sendPubRel(elm);
            elm->status = WAIT_PUBCOMP;
            elm->sendUTC = Timer::getUnixTime();
        }
    }else if (msg[0] == MQTTSN_TYPE_PUBCOMP){
        PubElement* elm = getElement(getUint16(msg + 1));
        if (elm == 0){
        	return;
        }
        if (elm->status == WAIT_PUBCOMP){
            remove(elm);  // PUBLISH Done
        }
    }
}

void PublishManager::published(uint8_t* msg, uint16_t msglen){
    if (msg[1] & MQTTSN_FLAG_QOS_1){
        sendPubAck(getUint16(msg + 2), getUint16(msg + 4), MQTTSN_RC_ACCEPTED);
    }
    Payload pl;
    pl.getPayload(msg + 6, msglen - 6);
    _publishedFlg = NEG_TASK_INDEX;
    theClient->getTopicTable()->execCallback(getUint16(msg + 2), &pl, msg[1] & 0x03);
    _publishedFlg = SAVE_TASK_INDEX;
}

void PublishManager::checkTimeout(void){
    PubElement* elm = _first;
	while (elm){
		if ( elm->sendUTC > 0 && elm->sendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
			if (elm->retryCount >= 0){
				sendPublish(elm);
				D_MQTTL("...Timeout retry\r\n");
				D_MQTTA(F("...Timeout retry\r\n"));
			}else{
				theClient->getGwProxy()->reconnect();
				elm->retryCount = MQTTSN_RETRY_COUNT;
				break;
			}
		}
		elm = elm->next;
	}
}


PubElement* PublishManager::getElement(uint16_t msgId){
    PubElement* elm = _first;
	while(elm){
		if (elm->msgId == msgId){
			return elm;
		}else{
			elm = elm->next;
		}
	}
	return 0;
}

PubElement* PublishManager::getElement(const char* topicName){
	PubElement* elm = _first;
	while(elm){
		if (strcmp(elm->topicName, topicName) == 0){
			return elm;
		}else{
			elm = elm->next;
		}
	}
	return 0;
}

void PublishManager::remove(PubElement* elm){
    if (elm){
        if (elm->prev == 0){
    		_first = elm->next;
    		if (elm->next != 0){
    			elm->next->prev = 0;
    		}
            delete elm->payload;
            delElement(elm);
    	}else{
    		elm->prev->next = elm->next;
    		delElement(elm);
    	}
        _elmCnt--;
    }
}

void PublishManager::delElement(PubElement* elm){
	if (elm->taskIndex >= 0){
		theClient->getTaskManager()->done(elm->taskIndex);
	}
	free(elm);
}


PubElement* PublishManager::add(const char* topicName, uint16_t topicId, Payload* payload, uint8_t qos, uint8_t retain, uint16_t msgId){
    PubElement* last = _first;
	PubElement* prev = _first;
	PubElement* elm = (PubElement*)calloc(1,sizeof(PubElement));

	if (elm == 0){
		return 0;
	}
	if (last == 0){
		_first = elm;
	}
    
    elm->topicName = topicName;

	if (strlen(topicName) == 2){
        topicId = 0;
        elm->flag |= MQTTSN_TOPIC_TYPE_SHORT;
    }else if (strlen(topicName) > 2){
        topicId = theClient->getTopicTable()->getTopicId((char*)topicName);
        elm->flag |= MQTTSN_TOPIC_TYPE_NORMAL;
    }else{
		elm->flag |= MQTTSN_TOPIC_TYPE_PREDEFINED;
	}
    if (qos == 0){
        elm->flag |= MQTTSN_FLAG_QOS_0;
    }else if ( qos == 1){
        elm->flag |= MQTTSN_FLAG_QOS_1;
    }else if (qos == 2){
        elm->flag |= MQTTSN_FLAG_QOS_2;
    }
    if (retain){
        elm->flag |= MQTTSN_FLAG_RETAIN;
    }

    if (topicId){
        elm->status = TOPICID_IS_READY;
        elm->topicId = topicId;
    }

    elm->payload = payload;
	elm->msgId = msgId;
	elm->retryCount = MQTTSN_RETRY_COUNT;
	elm->sendUTC = 0;

	if (_publishedFlg == NEG_TASK_INDEX){
		elm->taskIndex = -1;
	}else{
		elm->taskIndex = theClient->getTaskManager()->getIndex();
		theClient->getTaskManager()->suspend(elm->taskIndex);
	}

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
	++_elmCnt;
	return elm;
}
