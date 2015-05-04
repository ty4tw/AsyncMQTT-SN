/*
 * BrokerSendTask.cpp
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

#include "BrokerSendTask.h"
#include "GatewayResourcesProvider.h"
#include "GatewayDefines.h"
#include "lib/ProcessFramework.h"
#include "lib/Messages.h"
#include "lib/Defines.h"
#include <string.h>
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

extern char* currentDateTime();
using namespace tomyAsyncGateway;

BrokerSendTask::BrokerSendTask(GatewayResourcesProvider* res){
	_res = res;
	_res->attach(this);
}

BrokerSendTask::~BrokerSendTask(){
	if(_host){
		delete _host;
	}
	if(_service){
		delete _service;
	}
}

void BrokerSendTask::run(){
	Event* ev = 0;
	MQTTMessage* srcMsg = 0;
	ClientNode* clnode = 0;
	char param[TOMYFRAME_PARAM_MAX];

		if(_res->getParam("BrokerName",param) == 0){
			_host = strdup(param);
		}
		if(_res->getParam("BrokerPortNo",param) == 0){
			_service =strdup( param);
		}
		_light = _res->getLightIndicator();


	while(true){

		uint16_t length = 0;
		memset(_buffer, 0, SOCKET_MAXBUFFER_LENGTH);

		ev = _res->getBrokerSendQue()->wait();

		clnode = ev->getClientNode();
		srcMsg = clnode->getBrokerSendMessage();

		if(srcMsg->getType() == MQTT_TYPE_PUBLISH){
			MQTTPublish* msg = static_cast<MQTTPublish*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(BLUE_FORMAT, currentDateTime(), "PUBLISH", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_PUBACK){
			MQTTPubAck* msg = static_cast<MQTTPubAck*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(GREEN_FORMAT, currentDateTime(), "PUBACK", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_PUBREL){
			MQTTPubRel* msg = static_cast<MQTTPubRel*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(GREEN_FORMAT, currentDateTime(), "PUBREL", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_PINGREQ){
			MQTTPingReq* msg = static_cast<MQTTPingReq*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(FORMAT, currentDateTime(), "PINGREQ", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_SUBSCRIBE){
			MQTTSubscribe* msg = static_cast<MQTTSubscribe*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(FORMAT, currentDateTime(), "SUBSCRIBE", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_UNSUBSCRIBE){
			MQTTUnsubscribe* msg = static_cast<MQTTUnsubscribe*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(FORMAT, currentDateTime(), "UNSUBSCRIBE", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_CONNECT){
			MQTTConnect* msg = static_cast<MQTTConnect*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(FORMAT, currentDateTime(), "CONNECT", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			clnode->connectSended();
			send(clnode, length);

		}else if(srcMsg->getType() == MQTT_TYPE_DISCONNECT){
			MQTTDisconnect* msg = static_cast<MQTTDisconnect*>(srcMsg);
			length = msg->serialize(_buffer);
			LOGWRITE(FORMAT, currentDateTime(), "DISCONNECT", RIGHTARROW, GREEN_BROKER, msgPrint(msg));
			send(clnode, length);

			clnode->getStack()->disconnect();
		}

		delete ev;
	}
}


int BrokerSendTask::send(ClientNode* clnode, int length){
	int rc = -1;

	if(length <= 0){
		return rc;
	}

	if( clnode->getStack()->isValid()){
		rc = clnode->getStack()->send(_buffer, length);
		if(rc != length){
			LOGWRITE("\n%s   \x1b[0m\x1b[31merror:\x1b[0m\x1b[37m Can't Xmit to the Broker. errno=%d\n", currentDateTime(), rc == -1 ? errno : 0);
			clnode->getStack()->disconnect();
			clnode->disconnected();
		}else{
			_light->greenLight(true);
		}
	}else{
		if(clnode->getStack()->connect(_host, _service)){
			rc = clnode->getStack()->send(_buffer, length);
			if(rc != length){
				LOGWRITE("\n%s   \x1b[0m\x1b[31merror:\x1b[0m\x1b[37m Can't Xmit to the Broker. errno=%d\n", currentDateTime(), rc == -1 ? errno : 0);
				clnode->getStack()->disconnect();
				clnode->disconnected();
			}else{
				_light->greenLight(true);
			}
		}else{
			LOGWRITE("\n%s   \x1b[0m\x1b[31merror:\x1b[0m\x1b[37m Can't connect to the Broker.\n", currentDateTime());
			clnode->getStack()->disconnect();
			clnode->disconnected();
		}
	}
	return rc;
}

char*  BrokerSendTask::msgPrint(MQTTMessage* msg){
	char* buf = _printBuf;
	_light->blueLight(true);

	sprintf(buf, " %02X", *_buffer);
	buf += 3;

	for(int i = 0; i < msg->getRemainLength() + msg->getRemainLengthSize(); i++){
		sprintf(buf, " %02X", *( _buffer + 1  + i));
		buf += 3;
	}
	*buf = 0;
	_light->blueLight(false);
	return _printBuf;
}

