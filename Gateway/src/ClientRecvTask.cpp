/*
 * ClientRecvTask.cpp
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
#include "ClientRecvTask.h"
#include "GatewayResourcesProvider.h"
#include "GatewayDefines.h"
#include "lib/ProcessFramework.h"
#include "lib/Messages.h"
#include "lib/ZBStack.h"
#include "ErrorMessage.h"


#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

using namespace std;
using namespace tomyAsyncGateway;
extern char* currentDateTime();

ClientRecvTask::ClientRecvTask(GatewayResourcesProvider* res){
	_res = res;
	_res->attach(this);
}

ClientRecvTask::~ClientRecvTask(){

}



void ClientRecvTask::run(){
	NETWORK_CONFIG config;
	char param[TOMYFRAME_PARAM_MAX];
	bool secure = false;   // TCP

#ifdef NETWORK_XBEE

	if(_res->getParam("BaudRate",param) == 0){

		int val = atoi(param);
		switch(val){
		case 9600:
			config.baudrate = B9600;
			break;
		case 19200:
			config.baudrate =B19200;
			break;
		case 38400:
			config.baudrate =B38400;
			break;
		case 57600:
			config.baudrate =B57600;
			break;
		case 115200:
			config.baudrate = B115200;
			break;
		default:
			THROW_EXCEPTION(ExFatal, ERRNO_APL_01, "Invalid baud rate!");  // ABORT
		}
	}else{
		config.baudrate = B57600;
	}
	config.flag = O_RDONLY;
	if(_res->getParam("SerialDevice",param) == 0){
		config.device = strdup(param);
	}

	if(_res->getParam("SecureConnection",param) == 0){
		if(!strcasecmp(param, "YES")){
			secure = true;  // TLS
		}
	}

	_res->getClientList()->authorize(FILE_NAME_CLIENT_LIST, secure);
	_network = new Network();

#endif

#ifdef NETWORK_UDP
	if(_res->getParam("BroadcastIP", param) == 0){
		config.ipAddress = strdup(param);
	}
	if(_res->getParam("BroadcastPortNo",param) == 0){
		config.gPortNo = atoi(param);
	}
	if(_res->getParam("GatewayPortNo",param) == 0){
		config.uPortNo = atoi(param);
	}
	_network = _res->getNetwork();
#endif

#ifdef NETWORK_XXXXX
	_network = _res->getNetwork();
#endif

	if(_network->initialize(config) < 0){
		THROW_EXCEPTION(ExFatal, ERRNO_APL_01, "can't open the client port.");  // ABORT
	}

	uint8_t* recvMsg;
	int len;

	while(true){
		bool eventSetFlg = true;
		recvMsg = _network->Network::getResponce(&len);
		if (recvMsg != 0){
			uint8_t offset = 0;
			if (recvMsg[0] == 0x01){
				offset = 3;
			}else{
				offset = 1;
			}

			Event* ev = new Event();
			ClientNode* clnode = _res->getClientList()->getClient(_network->getAddrMsb(),_network->getAddrLsb(),
																  _network->getAddr16());

			if(!clnode){
				if(*(recvMsg + offset) == MQTTSN_TYPE_CONNECT){

				#ifdef NETWORK_XBEE
					ClientNode* node = _res->getClientList()->createNode(secure, _network->getAddrMsb(),_network->getAddrLsb(),0);
				#endif
				#ifdef NETWORK_UDP
					ClientNode* node = _res->getClientList()->createNode(secure, _network->getAddrMsb(),_network->getAddrLsb(),
																			_network->getAddr16());
				#endif
				#ifdef NETWORK_XXXXX
					ClientNode* node = _res->getClientList()->createNode(secure, _network->getAddrMsb(),_network->getAddrLsb(),
																			_network->getAddr16());
				#endif

					if(!node){
						delete ev;
						LOGWRITE("Client is not authorized.\n");
						continue;
					}

					MQTTSnConnect* msg = new MQTTSnConnect();
					msg->absorb(recvMsg);
					node->setClientAddress16(_network->getAddr16());
					if(msg->getClientId()->size() > 0){
						node->setNodeId(msg->getClientId());
					}
					node->setClientRecvMessage(msg);
					ev->setClientRecvEvent(node);
				}else if(*(recvMsg + offset) == MQTTSN_TYPE_SEARCHGW){
					MQTTSnSearchGw* msg = new MQTTSnSearchGw();
					msg->absorb(recvMsg);
					ev->setEvent(msg);

				}else{
					eventSetFlg = false;
				}
			}else{
				if (*(recvMsg + offset) == MQTTSN_TYPE_CONNECT){
					MQTTSnConnect* msg = new MQTTSnConnect();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					clnode->setClientAddress16(_network->getAddr16());
					ev->setClientRecvEvent(clnode);

				}else if(*(recvMsg + offset) == MQTTSN_TYPE_PUBLISH){
					MQTTSnPublish* msg = new MQTTSnPublish();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if(*(recvMsg + offset) == MQTTSN_TYPE_PUBACK){
					MQTTSnPubAck* msg = new MQTTSnPubAck();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if(*(recvMsg + offset) == MQTTSN_TYPE_PUBREL){
					MQTTSnPubRel* msg = new MQTTSnPubRel();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_CONNECT){
					MQTTSnConnect* msg = new MQTTSnConnect();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_PINGREQ){
					MQTTSnPingReq* msg = new MQTTSnPingReq();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_DISCONNECT){
					MQTTSnDisconnect* msg = new MQTTSnDisconnect();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_REGISTER){
					MQTTSnRegister* msg = new MQTTSnRegister();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_REGACK){
					MQTTSnRegAck* msg = new MQTTSnRegAck();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_UNSUBSCRIBE){
					MQTTSnUnsubscribe* msg = new MQTTSnUnsubscribe();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_SUBSCRIBE){
					MQTTSnSubscribe* msg = new MQTTSnSubscribe();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_WILLTOPIC){
					MQTTSnWillTopic* msg = new MQTTSnWillTopic();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if (*(recvMsg + offset) == MQTTSN_TYPE_WILLMSG){
					MQTTSnWillMsg* msg = new MQTTSnWillMsg();
					msg->absorb(recvMsg);
					clnode->setClientRecvMessage(msg);
					ev->setClientRecvEvent(clnode);

				}else if(*(recvMsg + offset) == MQTTSN_TYPE_SEARCHGW){
					MQTTSnSearchGw* msg = new MQTTSnSearchGw();
					clnode->disconnected();
					msg->absorb(recvMsg);
					ev->setEvent(msg);
				}else{
					eventSetFlg = false;
				}
			}
			if(eventSetFlg){
				_res->getGatewayEventQue()->post(ev);
			}else{
				delete ev;
			}
		}
	}
}





