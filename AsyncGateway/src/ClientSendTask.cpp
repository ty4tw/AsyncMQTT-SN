/*
 * ClientSendTask.cpp
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
#include "ClientSendTask.h"
#include "GatewayResourcesProvider.h"
#include "lib/ProcessFramework.h"
#include "lib/Messages.h"
#include "ErrorMessage.h"

#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

using namespace tomyAsyncGateway;

ClientSendTask::ClientSendTask(GatewayResourcesProvider* res){
	_res = res;
	_res->attach(this);
}

ClientSendTask::~ClientSendTask(){

}


void ClientSendTask::run(){
	NETWORK_CONFIG config;
#ifdef NETWORK_XBEE

	char param[TOMYFRAME_PARAM_MAX];
	bool secure = true;

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
	config.flag = O_WRONLY;
	if(_res->getParam("SerialDevice", param) == 0){
		config.device = strdup(param);
	}
	if(_res->getParam("SecureConnection",param) == 0){
		if(strcmp(param, "YES")){
			secure = false;  // TCP
		}
	}
	_res->getClientList()->authorize(FILE_NAME_CLIENT_LIST, secure);
	_network = new Network();

	if(_network->initialize(config) < 0){
		THROW_EXCEPTION(ExFatal, ERRNO_APL_01, "can't open the client port.");  // ABORT
	}

#endif

#ifdef NETWORK_UDP
	_network = _res->getNetwork();

#endif

#ifdef NETWORK_XXXXX
	_network = _res->getNetwork();
#endif


	while(true){
		Event* ev = _res->getClientSendQue()->wait();

		if(ev->getEventType() == EtClientSend){
			MQTTSnMessage msg = MQTTSnMessage();
			ClientNode* clnode = ev->getClientNode();
			msg.absorb( clnode->getClientSendMessage() );

			_network->unicast(msg.getMessagePtr(), msg.getMessageLength(),clnode->getMsb(), clnode->getLsb(), clnode->getAddress16());
		}else if(ev->getEventType() == EtBroadcast){
			MQTTSnMessage msg = MQTTSnMessage();
			msg.absorb( ev->getMqttSnMessage() );
			_network->broadcast(msg.getMessagePtr(), msg.getMessageLength());
		}
		delete ev;
	}
}


