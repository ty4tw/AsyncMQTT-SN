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
	_tkeepAlive = MQTTSN_DEFAULT_KEEPALIVE;
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
    _tkeepAlive = config.mqttsnCfg.keepAlive;
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
			writeGwMsg();
		}else if (_status == GW_SEND_WILLTOPIC){
			*pos++ = 3 + (uint8_t)strlen(_willTopic);
			*pos++ = MQTTSN_TYPE_WILLTOPIC;
			*pos++ = _qosWill | _retainWill;
			strcpy(pos,_willTopic);        // WILLTOPIC
			_status = GW_WAIT_WILLMSGREQ;
			writeGwMsg();
		}else if (_status == GW_CONNECTING || _status == GW_DISCONNECTED){
			uint8_t clientIdLen = uint8_t(strlen(_clientId) > 23 ? 23 : strlen(_clientId));
			*pos++ = 6 + clientIdLen;
			*pos++ = MQTTSN_TYPE_CONNECT;
			pos++;
			if (_cleanSession){
				_msg[2] = MQTTSN_FLAG_CLEAN;
			}
			*pos++ = MQTTSN_PROTOCOL_ID;
			setUint16((uint8_t*)pos, _tkeepAlive);
			pos += 2;
			strncpy(pos, _clientId, clientIdLen);
			_msg[ 6 + clientIdLen] = 0;
			if (_willMsg && _willTopic){
				_msg[2] = _msg[2] | MQTTSN_FLAG_WILL;   // CONNECT
				_status = GW_WAIT_WILLTOPICREQ;
			}else{
				_status = GW_WAIT_CONNACK;
			}
			writeGwMsg();
		}else if (_status == GW_LOST){
          #if defined(NETWORK_XBEE) || defined(BROADCAST_ENABLE)
			*pos++ = 3;
			*pos++ = MQTTSN_TYPE_SEARCHGW;
			*pos = 0;                        // SERCHGW
			_status = GW_SEARCHING;
			writeGwMsg();
          #else
			// UDP without multicast
            _status = GW_CONNECTING;
            _network.setFixedGwAddress();
		  #endif
		}
		getConnectResponce();
	}
	return;
}

int GwProxy::getConnectResponce(void){
	int len = readMsg();

	if (len == 0){
		if (_sendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
			if (--_retryCount > 0){
				writeMsg((const uint8_t*)_msg);  // Not writeGwMsg()
				_sendUTC = Timer::getUnixTime();
			}else{
				_sendUTC = 0;
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
		if (_mqttsnMsg[1] == 0x00){
			_status = GW_CONNECTED;
			_keepAliveTimer.start(_tkeepAlive * 1000);
			_topicTbl.clearTopic();
			theClient->onConnect();  // SUBSCRIBEs are conducted
		}else{
			_status = GW_CONNECTING;
		}
	}
	return 1;
}

void GwProxy::reconnect(void){
	D_MQTTL("...Gateway reconnect\r\n");
	_status = GW_DISCONNECTED;
	connect();
}

void GwProxy::disconnect(uint16_t secs){
    _tSleep = secs;
    _status = GW_DISCONNECTING;
    
	_msg[1] = MQTTSN_TYPE_DISCONNECT;

	if (secs){
		_msg[0] = 4;
		setUint16((uint8_t*) _msg + 2, secs);
	}else{
		_msg[0] = 2;
		_keepAliveTimer.stop();
	}

	_retryCount = MQTTSN_RETRY_COUNT;
	writeMsg((const uint8_t*)_msg);
	_sendUTC = Timer::getUnixTime();

	while ( _status != GW_DISCONNECTED && _status != GW_SLEPT){
		if (getDisconnectResponce() < 0){
			_status = GW_LOST;
			return;
		}
	}
}

int GwProxy::getDisconnectResponce(void){
	int len = readMsg();

	if (len == 0){
		if (_sendUTC + MQTTSN_TIME_RETRY < Timer::getUnixTime()){
			if (--_retryCount >= 0){
				writeMsg((const uint8_t*)_msg);
				_sendUTC = Timer::getUnixTime();
			}else{
				_status = GW_LOST;
				_gwId = 0;
				return -1;
			}
		}
		return 0;
	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_DISCONNECT){
		if (_tSleep){
			_status = GW_SLEEPING;
			_keepAliveTimer.start(_tSleep);
		}else{
			_status = GW_DISCONNECTED;
		}
	}
	return 0;
}

int GwProxy::getMessage(void){
	int len = readMsg();
	if (len < 0){
		return len;   //error
	}
#ifdef DEBUG_MQTTSN
	if (len){
		D_MQTTA(F(" recved msgType "));
		D_MQTTA(_mqttsnMsg[0], HEX);
		D_MQTTALN();
		D_MQTTL(" recved msgType %x\n", _mqttsnMsg[0]);
	}
#endif

	if (len == 0){
		// Check PINGREQ required
		checkPingReq();

		// Check ADVERTISE valid
		#if defined(NETWORK_XBEE) || defined(BROADCAST_ENABLE)
		checkAdvertise();
		#endif

		// Check Timeout of REGISTERs
		_regMgr.checkTimeout();

		// Check Timeout of PUBLISHes,
		theClient->getPublishManager()->checkTimeout();

		// Check Timeout of SUBSCRIBEs,
        theClient->getSubscribeManager()->checkTimeout();

	}else if (_mqttsnMsg[0] == MQTTSN_TYPE_PUBLISH){
        theClient->getPublishManager()->published(_mqttsnMsg, len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_PUBACK || _mqttsnMsg[0] == MQTTSN_TYPE_PUBCOMP ||
			_mqttsnMsg[0] == MQTTSN_TYPE_PUBREC || _mqttsnMsg[0] == MQTTSN_TYPE_PUBREL ){
        theClient->getPublishManager()->responce(_mqttsnMsg, (uint16_t)len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_SUBACK || _mqttsnMsg[0] == MQTTSN_TYPE_UNSUBACK){
        theClient->getSubscribeManager()->responce(_mqttsnMsg);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_REGISTER){
    	_regMgr.responceRegister(_mqttsnMsg, len);

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_REGACK){
        _regMgr.responceRegAck(getUint16(_mqttsnMsg + 3), getUint16(_mqttsnMsg + 1));

    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_PINGRESP){
        if (_pingStatus == GW_WAIT_PINGRESP){
            _pingStatus = 0;
            resetPingReqTimer();
        }
    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_DISCONNECT){
    	_status = GW_LOST;
    	_gwAliveTimer.stop();
    	_keepAliveTimer.stop();
    }else if (_mqttsnMsg[0] == MQTTSN_TYPE_ADVERTISE){
    	if (getUint16((const uint8_t*)(_mqttsnMsg + 2)) < 61){
    		_tAdv = getUint16((const uint8_t*)(_mqttsnMsg + 2)) * 1500;
    	}else{
    		_tAdv = getUint16((const uint8_t*)(_mqttsnMsg + 2)) * 1100;
    	}
        _gwAliveTimer.start(_tAdv);
    }
    return 0;
}




uint16_t GwProxy::registerTopic(char* topicName, uint16_t topicId){
    if (topicId){
    	_topicTbl.setTopicId("", topicId, MQTTSN_TOPIC_TYPE_PREDEFINED);
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
	//_status = GW_LOST;
	//_gwId = 0;
	return rc;
}

void GwProxy::writeGwMsg(void){
	_retryCount = MQTTSN_RETRY_COUNT;
	writeMsg((const uint8_t*)_msg);
	_sendUTC = Timer::getUnixTime();
}

int GwProxy::readMsg(void){
	int len = 0;
	_mqttsnMsg = _network.getMessage(&len);
	if (len == 0){
		return 0;
	}
	/*else if (len < 0){
		if ( _status < GW_CONNECTING){
			_status = GW_LOST;
		}else{
			_status = GW_CONNECTING;
		}
		return -1;
	}*/

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
                D_MQTTA(F("   !!! PINGREQ Timeout\n"));
                D_MQTTL("   !!! PINGREQ Timeout\n");
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
		D_MQTTA(F("   !!! ADVERTISE Timeout\n"));
		D_MQTTL("   !!! ADVERTISE Timeout\n");
	}
}

TopicTable* GwProxy::getTopicTable(void){
	return &_topicTbl;
}

RegisterManager* GwProxy::getRegisterManager(void){
	return &_regMgr;
}

void GwProxy::resetPingReqTimer(void){
	_keepAliveTimer.start(_tkeepAlive * 1000);
}
