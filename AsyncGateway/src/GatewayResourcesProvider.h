/*
 * GatewayResourcesProvider.h
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

#ifndef GATEWAY_RESOURCES_PROVIDER_H_
#define GATEWAY_RESOURCES_PROVIDER_H_

#include "lib/ProcessFramework.h"
#include "lib/Messages.h"
#include "lib/Topics.h"
#include "lib/TLSStack.h"
#include <map>

#define FILE_NAME_CLIENT_LIST "/usr/local/etc/tomygateway/config/clientList.conf"
#define DEFAULT_INFRAIGHT_MSG  10
using namespace tomyAsyncGateway;
/*=====================================
        Class MessageQue
 =====================================*/
template<class T> class MessageQue{
public:
	MessageQue();
	~MessageQue();
	T* getMessage();
	void push(T*);
	void pop();
	void clear();
private:
	queue<T*> _que;
	Mutex  _mutex;
};

/*=====================================
        Class TopicIdMap
 =====================================*/
typedef map<uint16_t,uint16_t> TOPICID_MAP;

class TopicIdMap{
public:
	TopicIdMap();
	~TopicIdMap();
	uint16_t getTopicId(uint16_t msgId);
	void add(uint16_t msgId, uint16_t topicId);
	void erase(uint16_t msgId);
	void clear(void);
private:
	TOPICID_MAP _map;
	Mutex       _mutex;
	uint8_t     _maxInflight;
};


enum ClientStatus {
	Cstat_Disconnected = 0,
	Cstat_TryConnecting,
	Cstat_Connecting,
	Cstat_Active,
	Cstat_Asleep,
	Cstat_Awake,
	Cstat_Lost
};

/*=====================================
        Class ClientNode
 =====================================*/
class ClientNode{
public:
	ClientNode();
	ClientNode(bool secure);
	~ClientNode();

	MQTTMessage*   getBrokerSendMessage();
	MQTTMessage*   getBrokerRecvMessage();
	MQTTSnMessage* getClientSendMessage();
	MQTTSnMessage* getClientRecvMessage();
	MQTTConnect*   getConnectMessage();
	uint16_t       getWaitedPubTopicId(uint16_t msgId);
	uint16_t       getWaitedSubTopicId(uint16_t msgId);
	MQTTSnMessage* getClientSleepMessage();
	void eraseWaitedPubTopicId(uint16_t msgId);
	void eraseWaitedSubTopicId(uint16_t msgId);
	void clearWaitedPubTopicId(void);
	void clearWaitedSubTopicId(void);

	void setBrokerSendMessage(MQTTMessage*);
	void setBrokerRecvMessage(MQTTMessage*);
	void setClientSendMessage(MQTTSnMessage*);
	void setClientRecvMessage(MQTTSnMessage*);
	void setConnectMessage(MQTTConnect*);
	void setWaitedPubTopicId(uint16_t msgId, uint16_t topicId);
	void setWaitedSubTopicId(uint16_t msgId, uint16_t topicId);
	void setClientSleepMessage(MQTTSnMessage*);

	void deleteBrokerSendMessage();
	void deleteBrokerRecvMessage();
	void deleteClientSendMessage();
	void deleteClientRecvMessage();

	void checkTimeover();
	void updateStatus(MQTTSnMessage*);
	void updateStatus(ClientStatus);
	void connectSended();
	void connackSended(int rc);
	void connectQued();
	void disconnected();
	bool isConnectSendable();
	uint16_t getNextMessageId();
	uint8_t  getNextSnMsgId();
	Topics*  getTopics();

	TLSStack* getStack();
	uint32_t  getMsb(void);
	uint32_t  getLsb(void);
	uint16_t  getAddress16();
	string*   getNodeId();
	void setMsb(uint32_t);
	void setLsb(uint32_t);
	void setClientAddress16(uint16_t addr);
	void setClientAddress64(uint32_t msb, uint32_t lsb);
	void setTopics(Topics* topics);
	void setNodeId(string* id);
	int  checkConnAck(MQTTSnConnack* msg);
	MQTTSnConnack*  checkGetConnAck();
	void setConnAckSaveFlg();
	void setWaitWillMsgFlg();
	bool isDisconnect();
	bool isActive();
	bool isSleep();

private:
	void setKeepAlive(MQTTSnMessage* msg);

	MessageQue<MQTTMessage>   _brokerSendMessageQue;
	MessageQue<MQTTMessage>   _brokerRecvMessageQue;
	MessageQue<MQTTSnMessage> _clientSendMessageQue;
	MessageQue<MQTTSnMessage> _clientRecvMessageQue;
	MessageQue<MQTTSnMessage> _clientSleepMessageQue;

	MQTTConnect*   _mqttConnect;

	TopicIdMap  _waitedPubTopicIdMap;
	TopicIdMap  _waitedSubTopicIdMap;

	uint16_t _msgId;
	uint8_t  _snMsgId;
	Topics*  _topics;
	ClientStatus _status;
	uint32_t _keepAliveMsec;
	Timer    _keepAliveTimer;

	TLSStack* _stack;

	uint32_t _msb;
	uint32_t _lsb;
    uint16_t _addr16;
    string   _nodeId;
    bool _connAckSaveFlg;
    bool _waitWillMsgFlg;
    MQTTSnConnack*  _connAck;

};

/*=====================================
        Class ClientList
 =====================================*/
class ClientList{
public:
	ClientList();
	~ClientList();
	void authorize(const char* fileName, bool secure);
	void erase(ClientNode*);
	ClientNode* getClient(uint32_t msb, uint32_t lsb, uint16_t addr16);
	ClientNode* createNode(bool secure, uint32_t msb, uint32_t lsb, uint16_t addr16, string* nodeId = 0);
	uint16_t getClientCount();
	ClientNode* operator[](int);
	bool isAuthorized();
private:
	vector<ClientNode*>*  _clientVector;
	Mutex _mutex;
	uint16_t _clientCnt;
	bool _authorize;
};

/*=====================================
         Class Event
  ====================================*/
enum EventType{
	Et_NA = 0,
	EtTimeout,

	EtBrokerSend,
	EtBrokerRecv,
	EtClientSend,
	EtClientRecv,
	EtBroadcast,
	EtSocketAlive
};

class Event{
public:
	Event();
	Event(EventType);
	~Event();
	EventType getEventType();
	void setClientSendEvent(ClientNode*);
	void setBrokerSendEvent(ClientNode*);
	void setClientRecvEvent(ClientNode*);
	void setBrokerRecvEvent(ClientNode*);
	void setEvent(MQTTSnMessage*);
	void setTimeout();
	ClientNode* getClientNode();
	MQTTSnMessage* getMqttSnMessage();
private:
	EventType   _eventType;
	ClientNode* _clientNode;
	MQTTSnMessage* _mqttSnMessage;
};

/*=====================================
     Class LightIndicator
 =====================================*/
class LightIndicator{
public:
	LightIndicator();
	~LightIndicator();
	void greenLight(bool);
	void blueLight(bool);
	void redLightOff();
private:
	void init();
	void lit(int gpioNo, int onoff);
	bool _greenStatus;
	bool _blueStatus;
	bool _gpioAvailable;
};

/*=====================================
     Class GatewayResourcesProvider
 =====================================*/
class GatewayResourcesProvider: public MultiTaskProcess{
public:
	GatewayResourcesProvider();
	~GatewayResourcesProvider();

	EventQue<Event>* getGatewayEventQue();
	EventQue<Event>* getClientSendQue();
	EventQue<Event>* getBrokerSendQue();
	ClientList* getClientList();
	Network* getNetwork();
	LightIndicator* getLightIndicator();
private:
	ClientList _clientList;
	EventQue<Event> _gatewayEventQue;
	EventQue<Event> _brokerSendQue;
	EventQue<Event> _clientSendQue;
	Network _network;
	LightIndicator _lightIndicator;
};



/*=====================================
    Class MessageQue Implementation
 =====================================*/
template<class T> MessageQue<T>::MessageQue(){

}

template<class T> MessageQue<T>::~MessageQue(){
	clear();
}

template<class T> T* MessageQue<T>::getMessage(){
	T* msg;
	if(!_que.empty()){
		_mutex.lock();
		msg = _que.front();
		_mutex.unlock();
		return msg;
	}else{
		return 0;
	}
}

template<class T> void MessageQue<T>::push(T* msg){
	_mutex.lock();
	_que.push(msg);
	_mutex.unlock();
}

template<class T> void MessageQue<T>::pop(){
	if(!_que.empty()){
		_mutex.lock();
		delete _que.front();
		_que.pop();
		_mutex.unlock();
	}
}

template<class T> void MessageQue<T>::clear(){
	_mutex.lock();
	while(!_que.empty()){
		delete _que.front();
		_que.pop();
	}
	_mutex.unlock();
}

#endif /* GATEWAY_RESOURCES_PROVIDER_H_ */
