/*
 * GwProxy.h
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

#ifndef GWPROXY_H_
#define GWPROXY_H_

#ifdef ARDUINO
	#include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#ifdef ARDUINO
  #ifdef NETWORK_UDP
    #include <NetworkUdp.h>
  #endif
  #ifdef NETWORK_XBEE
    #include <NetworkXBee.h>
  #endif
  #include <Timer.h>
  #include <RegisterManager.h>
  #include <TopicTable.h>

  #if defined(DEBUG_NW) || defined(DEBUG_MQTTSN) || defined(DEBUG)
        #include <SoftwareSerial.h>
        extern SoftwareSerial debug;
  #endif

#endif  /* ARDUINO */


#ifdef LINUX
  #ifdef NETWORK_UDP
    #include "NetworkUdp.h"
  #endif
  #ifdef NETWORK_XBEE
    #include "NetworkXBee.h"
  #endif
  #include "Timer.h"
  #include "RegisterManager.h"
  #include "TopicTable.h"
  #include <stdio.h>
  #include <string.h>
#endif /* LINUX */

using namespace std;

#define GW_LOST              0
#define GW_SEARCHING         1
#define GW_CONNECTING        2
#define GW_WAIT_WILLTOPICREQ 3
#define GW_SEND_WILLTOPIC    4
#define GW_WAIT_WILLMSGREQ   5
#define GW_SEND_WILLMSG      6
#define GW_WAIT_CONNACK      7
#define GW_CONNECTED         8
#define GW_DISCONNECTING     9
#define GW_SLEEPING         10
#define GW_DISCONNECTED     11
#define GW_SLEPT            12

#define GW_WAIT_PINGRESP     1

namespace tomyAsyncClient {
/*========================================
       Class GwProxy
 =======================================*/
class GwProxy{
public:
	GwProxy();
	~GwProxy();

	void     initialize(APP_CONFIG config);
	void     connect(void);
	void     disconnect(uint16_t sec = 0);
	int      getMessage(void);
	uint16_t registerTopic(char* topic, uint16_t toipcId);

	void     setWillTopic(const char* willTopic, uint8_t qos, bool retain = false);
	void     setWillMsg(const char* willMsg);
	void     setCleanSession(bool);
	void     setKeepAliveDuration(uint16_t duration);
	void     setAdvertiseDuration(uint16_t duration);
	void     reconnect(void);
	int      writeMsg(const uint8_t* msg);
	void     resetPingReqTimer(void);
	uint16_t getNextMsgId();
	TopicTable* getTopicTable(void);
	RegisterManager* getRegisterManager(void);
private:
	int      readMsg(void);
	void     writeGwMsg(void);
	void     checkPingReq(void);
	void     checkAdvertise(void);
	int      getConnectResponce(void);
	int      getDisconnectResponce(void);

	Network     _network;
	uint8_t*    _mqttsnMsg;
	uint16_t    _nextMsgId;
	const char* _clientId;
	const char* _willTopic;
	const char* _willMsg;
	uint8_t     _cleanSession;
	uint8_t     _retainWill;
	uint8_t     _qosWill;
	uint8_t     _gwId;
	uint16_t    _tkeepAlive;
	uint32_t    _tAdv;
	uint32_t    _sendUTC;
	int         _retryCount;
	uint8_t     _status;
	uint32_t    _pingSendUTC;
	uint8_t     _pingRetryCount;
	uint8_t     _pingStatus;
	RegisterManager _regMgr;
	TopicTable  _topicTbl;
	Timer       _gwAliveTimer;
	Timer       _keepAliveTimer;
	uint16_t    _tSleep;
	char        _msg[MQTTSN_MAX_MSG_LENGTH + 1];
};



} /* tomyAsyncClient */
#endif /* GWPROXY_H_ */
