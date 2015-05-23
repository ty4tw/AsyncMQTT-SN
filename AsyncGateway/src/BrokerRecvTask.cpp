/*
 * BrokerRecvTask.cpp
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

#include "BrokerRecvTask.h"
#include "GatewayResourcesProvider.h"
#include "GatewayDefines.h"
#include "lib/ProcessFramework.h"
#include "lib/Messages.h"
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

using namespace tomyAsyncGateway;
extern char* currentDateTime();


BrokerRecvTask::BrokerRecvTask(GatewayResourcesProvider* res){
	_res = res;
	_res->attach(this);
	_stableNetwork = true;
}

BrokerRecvTask::~BrokerRecvTask(){

}


void BrokerRecvTask::run(){
	char param[TOMYFRAME_PARAM_MAX];
	if(_res->getParam("NetworkIsStable",param) == 0){
		if(!strcasecmp(param, "NO")){
			_stableNetwork = false;
		}
	}

	struct timeval timeout;

	LightIndicator* lightIndicator = _res->getLightIndicator();
	ClientList* clist = _res->getClientList();
	fd_set rset;
	fd_set wset;

	while(true){
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;    // 500 msec
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		int maxSock = 0;
		int sockfd = 0;

		/*------- Prepare socket list to check -------*/
		for( int i = 0; i < clist->getClientCount(); i++){
			if((*clist)[i]){
				if((*clist)[i]->getStack()->isValid()){
					sockfd = (*clist)[i]->getStack()->getSock();
					FD_SET(sockfd, &rset);
					FD_SET(sockfd, &wset);
					if(sockfd > maxSock){
						maxSock = sockfd;
					}
				}
			}else{
				break;
			}
		}

		if(maxSock == 0){
			lightIndicator->greenLight(false);
		}else{

			/*------- Check socket to receive -------*/
			int activity =  select( maxSock + 1 , &rset , 0 , 0 , &timeout);

			if (activity > 0){
				for( int i = 0; i < clist->getClientCount(); i++){
					if((*clist)[i]){
						if((*clist)[i]->getStack()->isValid()){
							int sockfd = (*clist)[i]->getStack()->getSock();
							if(FD_ISSET(sockfd, &rset)){
								recvAndFireEvent((*clist)[i]);
								lightIndicator->greenLight(true);
							}
						}
					}else{
						break;
					}
				}
			}
		}
	}
}


/*-----------------------------------------
     Recv socket & Create MQTT Messages
 -----------------------------------------*/
void BrokerRecvTask::recvAndFireEvent(ClientNode* clnode){

	uint8_t sbuff[SOCKET_MAXBUFFER_LENGTH * 5];
	uint8_t buffer[SOCKET_MAXBUFFER_LENGTH];
	memset(buffer, 0, SOCKET_MAXBUFFER_LENGTH);
	int recvLength = 0;
	uint8_t* packet = buffer;

	recvLength = clnode->getStack()->recv(packet,SOCKET_MAXBUFFER_LENGTH);

	if (recvLength == -1){
		LOGWRITE(" Client : %s Error: BrokerRecvTask can't Receive data from Broker\n", clnode->getNodeId()->c_str());
		clnode->disconnected();
	}

	while(recvLength > 0){

		if((*packet & 0xf0) == MQTT_TYPE_PUBACK){
			MQTTPubAck* puback = new MQTTPubAck();
			puback->deserialize(packet);
			puback->serialize(sbuff);
			LOGWRITE(BLUE_FORMAT1, currentDateTime(), "PUBACK", LEFTARROW, BROKER, msgPrint(sbuff, puback));

			clnode->setBrokerRecvMessage(puback);
		}else if((*packet & 0xf0) == MQTT_TYPE_PUBREC){
			MQTTPubRec* pubRec = new MQTTPubRec();
			pubRec->deserialize(packet);
			pubRec->serialize(sbuff);
			LOGWRITE(BLUE_FORMAT1, currentDateTime(), "PUBREC", LEFTARROW, BROKER, msgPrint(sbuff, pubRec));

			clnode->setBrokerRecvMessage(pubRec);

		}else if((*packet & 0xf0) == MQTT_TYPE_PUBREL){
			MQTTPubRel* pubRel = new MQTTPubRel();
			pubRel->deserialize(packet);
			pubRel->serialize(sbuff);
			LOGWRITE(BLUE_FORMAT1, currentDateTime(), "PUBREL", LEFTARROW, BROKER, msgPrint(sbuff, pubRel));

			clnode->setBrokerRecvMessage(pubRel);

		}else if((*packet & 0xf0) == MQTT_TYPE_PUBCOMP){
			MQTTPubComp* pubComp = new MQTTPubComp();
			pubComp->deserialize(packet);
			pubComp->serialize(sbuff);
			LOGWRITE(BLUE_FORMAT1, currentDateTime(), "PUBCOMP", LEFTARROW, BROKER, msgPrint(sbuff, pubComp));

			clnode->setBrokerRecvMessage(pubComp);

		}else if((*packet & 0xf0) == MQTT_TYPE_PUBLISH){
			MQTTPublish* publish = new MQTTPublish();
			if(!publish->deserialize(packet)){
				clnode->disconnected();
				clnode->getStack()->disconnect();
				LOGWRITE("%s ill-formed UTF-8\n",currentDateTime());
			}
			publish->serialize(sbuff);
			if (publish->isDup()){
				LOGWRITE(GREEN_FORMAT2, currentDateTime(), "PUBLISH +", LEFTARROW, BROKER, msgPrint(sbuff, publish));
			} else {
				LOGWRITE(GREEN_FORMAT2, currentDateTime(), "PUBLISH", LEFTARROW, BROKER, msgPrint(sbuff, publish));
			}
			clnode->setBrokerRecvMessage(publish);

		}else if((*packet & 0xf0) == MQTT_TYPE_SUBACK){
			MQTTSubAck* suback = new MQTTSubAck();
			suback->deserialize(packet);
			suback->serialize(sbuff);
			LOGWRITE(FORMAT1, currentDateTime(), "SUBACK", LEFTARROW, BROKER, msgPrint(sbuff, suback));

			clnode->setBrokerRecvMessage(suback);

		}else if((*packet & 0xf0) == MQTT_TYPE_PINGRESP){
			MQTTPingResp* pingresp = new MQTTPingResp();
			pingresp->deserialize(packet);
			pingresp->serialize(sbuff);
			LOGWRITE(FORMAT1, currentDateTime(), "PINGRESP", LEFTARROW, BROKER, msgPrint(sbuff, pingresp));

			clnode->setBrokerRecvMessage(pingresp);

		}else if((*packet & 0xf0) == MQTT_TYPE_UNSUBACK){
			MQTTUnsubAck* unsuback = new MQTTUnsubAck();
			unsuback->deserialize(packet);
			unsuback->serialize(sbuff);
			LOGWRITE(FORMAT1, currentDateTime(), "UNSUBACK", LEFTARROW, BROKER, msgPrint(sbuff, unsuback));

			clnode->setBrokerRecvMessage(unsuback);

		}else if((*packet & 0xf0) == MQTT_TYPE_CONNACK){
			MQTTConnAck* connack = new MQTTConnAck();
			connack->deserialize(packet);
			connack->serialize(sbuff);
			LOGWRITE(CYAN_FORMAT1, currentDateTime(), "CONNACK", LEFTARROW, BROKER, msgPrint(sbuff, connack));

			clnode->setBrokerRecvMessage(connack);

		}else{
			LOGWRITE("%s UNKOWN_TYPE  packetLength=%d\n",currentDateTime(), recvLength);
			return;
		}

		Event* ev = new Event();
		ev->setBrokerRecvEvent(clnode);
		_res->getGatewayEventQue()->post(ev);

		RemainingLength rl;
		rl.deserialize(packet + 1);
		uint16_t  packetLength = 1 + rl.getSize() + rl.decode();
		recvLength -= packetLength;
		packet += packetLength;
	}
}


char*  BrokerRecvTask::msgPrint(uint8_t* buffer, MQTTMessage* msg){
	char* buf = _printBuf;

	_res->getLightIndicator()->blueLight(true);

	sprintf(buf, " %02X", *buffer);
	buf += 3;

	for(int i = 0; i < msg->getRemainLength() + msg->getRemainLengthSize(); i++){
		sprintf(buf, " %02X", *( buffer + 1 + i));
		buf += 3;
	}
	*buf = 0;
	_res->getLightIndicator()->blueLight(false);
	return _printBuf;
}
