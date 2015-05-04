/*
 * GwProxy.cpp
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
 *  Created on: 2015/04/19
 *    Modified: 
 *      Author: tomoaki
 *     Version: 0.0.1
 */

#ifdef ARDUINO
        #include <MqttsnClientApp.h>
        #include <MqttsnClient.h>
        #include <GwProxy.h>
#else
        #include "MqttsnClientApp.h"
        #include "MqttsnClient.h"
        #include "GwProxy.h"
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

using namespace std;
using namespace tomyAsyncClient;

extern void setUint16(uint8_t* pos, uint16_t val);
extern uint16_t getUint16(const uint8_t* pos);
extern MqttsnClient* theClient;
/*=====================================
        Class GwProxy
 ======================================*/
GwProxy::GwProxy(){
	_nextMsgId = 0;
	_status = GW_LOST;
    _gwId = 0;
	_willTopic = 0;
	_willMsg = 0;
	_qosWill = 0;
	_retainWill = 0;
	_tPing = MQTTSN_DEFAULT_KEEPALIVE;
	_tAdv = MQTTSN_DEFAULT_DURATION;
	_cleanSession = 0;
	_pingStatus = 0;
}

GwProxy::~GwProxy(){

}

void GwProxy::initialize(APP_CONFIG config){
	_network.initialize(config.netCfg);
	_clientId = config.netCfg.clientId;
    _willTopic = config.mqttsnCfg.willTopic;
    _willMsg = config.mqttsnCfg.willMsg;
    _qosWill = config.mqttsnCfg.willQos;
    _retainWill = config.mqttsnCfg.willRetain;
    _cleanSession = config.mqttsnCfg.cleanSession;
    _tPing = config.mqttsnCfg.keepAlive * 1000;
}

void GwProxy::connect(){
	char* pos;

	while (_status != GW_CONNECTED){
		pos = _msg;
		if (_status == GW_SEND_WILLMSG){
			*pos++ = 2 + (uint8_t)strlen(_willMsg);
			*pos++ = MQTTSN_TYPE_WILLMSG;
			strcpy(pos,_willMsg);          // WILLMSG
			_status = GW_WAIT_CONNACK;
		}else if (_status == GW_SEND_WILLTOPIC){
			*pos++ = 3 + (uint8_t)strlen(_willTopic);
			*pos++ = MQTTSN_TYPE_WILLTOPIC;
			*pos++ = _qosWill | _retainWill;
			strcpy(pos,_willTopic);        // WILLTOPIC
			_status = GW_WAIT_WILLMSGREQ;
		}else if (_status == GW_CONNECTING){
			*pos++ = 6 + (uint8_t)strlen(_clientId);
			*pos++ = MQTTSN_TYPE_CONNECT;
			pos++;
			if (_cleanSession){
				_msg[2] = MQTTSN_FLAG_CLEAN;
			}
			*pos++ = MQTTSN_PROTOCOL_ID;
			setUint16((uint8_t*)pos, _tPing);
			pos += 2;
			strncpy(pos, _clientId, strlen(_clientId));
			if (_willMsg && _willTopic){
				_msg[2] = _msg[2] | MQTTSN_FLAG_WILL;   // CONNECT
				_status = GW_WAIT_WILLTOPICREQ;
			}else{
				_status = GW_WAIT_CONNACK;
			}
		}else if (_status == GW_LOST){
          #if defined(NETWORK_XBEE) || defined(BROADCAST_ENABLE)
			*pos++ = 3;
			*pos++ = MQTTSN_TYPE_SEARCHGW;
			*pos = 0;                        // SERCHGW
			_status = GW_SEARCHING;
          #else
            _status = GW_CONNECTING;
            _network.setFixedGwAddress();
		  #endif
		}else{
			if (getConnectResponce() < 0){
				_status = GW_LOST;
			}
			continue;
		}
		_retryCount = MQTTSN_RETRY_COUNT;
		writeMsg((const uint8_t*)_msg);
		_sendUTC = Timer::getUnixTime();
	}

	_keepAliveTimer.start(_tPing);
	return;
}

int GwProxy::getConnectResponce(void){
	int len = readMsg();

	if (len == 0){
		if (_sendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
			if (--_retryCount > 0){
				writeMsg((const uint8_t*)_msg);
				_sendUTC = Timer::getUnixTime();
			}else{
				if (_status > GW_SEARCHING){
					_status = GW_CONNECTING;
				}else{
					_status = GW_LOST;
					_gwId = 0;
				}
				return -1;
			}
		}
		return 0;
	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_GWINFO && _status == GW_SEARCHING){
		_network.setGwAddress();
        _gwId = _mqttsnMsg[1];
		_status = GW_CONNECTING;
	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_WILLTOPICREQ && _status == GW_WAIT_WILLTOPICREQ){
		_status = GW_SEND_WILLTOPIC;
	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_WILLMSGREQ && _status == GW_WAIT_WILLMSGREQ){
		_status = GW_SEND_WILLMSG;
	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_CONNACK && _status == GW_WAIT_CONNACK){
printf("CONNACK recv\n");
		if (_mqttsnMsg[1] == 0x00){
			_status = GW_CONNECTED;
		}else{
			_status = GW_CONNECTING;
		}
	}
	return 0;
}


void GwProxy::disconnect(uint16_t secs){
	uint8_t msg[4];
    
	msg[1] = MQTTSN_TYPE_DISCONNECT;

	if (secs){
		msg[0] = 4;
		setUint16(msg + 2, secs);
		_keepAliveTimer.start(secs);
	}else{
		msg[0] = 2;	
		_keepAliveTimer.stop();
	}
	writeMsg(msg);
}


int GwProxy::getResponce(void){
	int len = readMsg();
	if (len < 0){
		return len;   //error
	}
	if (len){
		printf("Recv MQTT-SN TYPE=0x%x\n", _mqttsnMsg[0]);
	}

	if (len == 0){
		checkPingReq();    // Check PINGREQ required

		#if defined(NETWORK_XBEE) || defined(BROADCAST_ENABLE)
		checkAdvertise();  // Check ADVERTISE
		#endif

		_regMgr.checkTimeout();
		theClient->getPublishManager()->checkTimeout();
        theClient->getSubscribeManager()->checkTimeout();

	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_PUBACK || _mqttsnMsg[0] == MQTTSN_TYPE_PUBCOMP ||
			_mqttsnMsg[0] == MQTTSN_TYPE_PUBREC || _mqttsnMsg[0] == MQTTSN_TYPE_PUBREL ){
        theClient->getPublishManager()->responce(_mqttsnMsg, (uint16_t)len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_PUBLISH){
        theClient->getPublishManager()->published(_mqttsnMsg, len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_SUBACK || _mqttsnMsg[0] == MQTTSN_TYPE_UNSUBACK){
        theClient->getSubscribeManager()->responce(_mqttsnMsg);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_REGISTER){
    	_regMgr.responceRegister(_mqttsnMsg, len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_REGACK){
        _regMgr.responceRegAck(getUint16(_mqttsnMsg + 3), getUint16(_mqttsnMsg + 1));

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_PINGRESP){
        if (_pingStatus == GW_WAIT_PINGRESP){
            _pingStatus = 0;
            _keepAliveTimer.start(_tPing);
        }
    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_DISCONNECT){
        // ToDo: DISCONNECT
    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_ADVERTISE){
    	_tAdv = getUint16((const uint8_t*)(_mqttsnMsg + 2)) * 1000;
        _gwAliveTimer.start(_tAdv);
    }
    return 0;
}




uint16_t GwProxy::registerTopic(const char* topicName, uint16_t topicId){
    if (topicId){
        // ToDo  Predefined Topic
    }else{
        uint16_t topicId = _topicTbl.getTopicId(topicName);
        if (topicId == 0){
            _regMgr.registerTopic(topicName);
        }
    }
	return topicId;
}


int GwProxy::writeMsg(const uint8_t* msg){
	uint16_t len;
	uint8_t  rc;

	if (msg[0] == 0x01){
		len = getUint16(msg + 1);
	}else{
		len = msg[0];
	}

	if (msg[0] == 3 && msg[1] == MQTTSN_TYPE_SEARCHGW){
		rc = _network.broadcast(msg,len);
	}else{
		rc = _network.unicast(msg,len);
	}

	if (rc > 0){
		return rc;
	}
	_status = GW_LOST;
	_gwId = 0;
	return rc;
}

int GwProxy::readMsg(void){
	int len = 0;
	_mqttsnMsg = _network.getResponce(&len);
	if (len == 0){
		return 0;
	}else if (len < 0){
		if ( _status < GW_CONNECTING){
			_status = GW_LOST;
		}else{
			_status = GW_CONNECTING;
		}
		return -1;
	}
	if (_mqttsnMsg[0] == 0x01){
		int msgLen = (int) getUint16((const uint8_t*)_mqttsnMsg + 1);
		if (len != msgLen){
			_mqttsnMsg += 3;
			len = msgLen - 3;
		}
	}else{
		_mqttsnMsg += 1;
		len -= 1;
	}
	return len;
}

void GwProxy::setWillTopic(const char* willTopic, uint8_t qos, bool retain){
	_willTopic = willTopic;
	_retainWill = _qosWill = 0;
	if (qos == 1){
		_qosWill = MQTTSN_FLAG_QOS_1;
	}else if (qos == 2){
		_qosWill = MQTTSN_FLAG_QOS_2;
	}
	if (retain){
		_retainWill = MQTTSN_FLAG_RETAIN;
	}
}
void GwProxy::setWillMsg(const char* willMsg){
	_willMsg = willMsg;
}


void GwProxy::setCleanSession(bool flg){
	if (flg){
		_cleanSession = MQTTSN_FLAG_CLEAN;
	}else{
		_cleanSession = 0;
	}
}

uint16_t GwProxy::getNextMsgId(void){
	_nextMsgId++;
    if (_nextMsgId == 0){
    	_nextMsgId = 1;
    }
    return _nextMsgId;
}

void GwProxy::checkPingReq(void){
    uint8_t msg[2];
	msg[0] = 0x02;
	msg[1] = MQTTSN_TYPE_PINGREQ;
    
	if (_status == GW_CONNECTED && _keepAliveTimer.isTimeUp() && _pingStatus != GW_WAIT_PINGRESP){
		_pingStatus = GW_WAIT_PINGRESP;
        _pingRetryCount = MQTTSN_RETRY_COUNT;
		writeMsg((const uint8_t*)msg);
        _pingSendUTC = Timer::getUnixTime();
	}else if (_pingStatus == GW_WAIT_PINGRESP){
        if (_pingSendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
    		if (--_pingRetryCount > 0){
				writeMsg((const uint8_t*)_msg);
				_pingSendUTC = Timer::getUnixTime();
			}else{
				_status = GW_LOST;
				_gwId = 0;
                _pingStatus = 0;
                _keepAliveTimer.stop();
			}
		}
	}
}

void GwProxy::checkAdvertise(void){
	if ( _gwAliveTimer.isTimeUp()){
		_status = GW_LOST;
		_gwId = 0;
		_pingStatus = 0;
		_gwAliveTimer.stop();
		_keepAliveTimer.stop();
	}
}

TopicTable* GwProxy::getTopicTable(void){
	return &_topicTbl;
}

RegisterManager* GwProxy::getRegisterManager(void){
	return &_regMgr;
}
