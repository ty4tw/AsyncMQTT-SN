/*
 * XXXXXStack.cpp
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

#include "Defines.h"

#ifdef NETWORK_XXXXX

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "ProcessFramework.h"
#include "XXXXXStack.h"

using namespace std;
using namespace tomyAsyncGateway;

extern uint16_t getUint16(uint8_t* pos);
extern uint32_t getUint32(uint8_t* pos);
extern void setUint16(uint8_t* pos, uint16_t val);
extern void setUint32(uint8_t* pos, uint32_t val);

/*=========================================
       Class Network
 =========================================*/
Network::Network(){
}

Network::~Network(){

}

void Network::unicast(NWAddress64* addr64, uint16_t addr16, uint8_t* payload, uint16_t payloadLength){
	XXXXXPort::unicast( );
}

void Network::broadcast(uint8_t* payload, uint16_t payloadLength){
	XXXXXPort::multicast( );
}

bool Network::getResponse(NWResponse* response){

}

int Network::initialize(XXXXXConfig  config){
	return XXXXXPort::initialize(config);
}


/*=========================================
       Class XXXXXPort
 =========================================*/

XXXXXPort::XXXXXPort(){


}

XXXXXPort::~XXXXXPort(){
    close();
}

void XXXXXPort::close(){

}

int XXXXXPort::initialize(){
	return initialize(_config);
}

int XXXXXPort::initialize(UdpConfig config){

	_config.param1 = config.param1;
	_config.param2 = config.param2;
	_config.param3 = config.param3;


}


int XXXXXPort::unicast( ){

}

int XXXXXPort::multicast(){

}

int XXXXXPort::recv(){

}


/*=========================================
             Class NLLongAddress
 =========================================*/
NWAddress64::NWAddress64(){
    _msb = _lsb = 0;
}

NWAddress64::NWAddress64(uint32_t msb, uint32_t lsb){
    _msb = msb;
    _lsb = lsb;
}

uint32_t NWAddress64::getMsb(){
    return _msb;
}

uint32_t NWAddress64::getLsb(){
    return _lsb;
}

void NWAddress64::setMsb(uint32_t msb){
    _msb = msb;
}

void NWAddress64::setLsb(uint32_t lsb){
    _lsb = lsb;
}

bool NWAddress64::operator==(NWAddress64& addr){
	if(_msb == addr.getMsb() && _lsb == addr.getLsb()){
		return true;
	}else{
		return false;
	}
}

/*=========================================
             Class ZBResponse
 =========================================*/
NWResponse::NWResponse(){
    _addr16 = 0;
    memset( _frameDataPtr, 0, MQTTSN_MAX_FRAME_SIZE);
}

uint8_t  NWResponse::getFrameLength(){
	return _len;
}

void NWResponse::setLength(uint16_t len){
	_len = len;
}

NWAddress64*  NWResponse::getClientAddress64(){
    return &_addr64;
}

uint16_t NWResponse::getClientAddress16(){
  return _addr16;
}

void  NWResponse::setClientAddress64(uint32_t msb, uint32_t lsb){
    _addr64.setMsb(msb);
    _addr64.setLsb(lsb);
}

void  NWResponse::setClientAddress16(uint16_t addr16){
	_addr16 = addr16;
}

void NWResponse::setMsgType(uint8_t type){
	_type = type;
}


uint8_t NWResponse::getMsgType(){
	if(_len > 255){
		return _frameDataPtr[3];
	}else{
		return _frameDataPtr[1];
	}
}

uint8_t* NWResponse::getBody(){
	if(_len > 255){
		return _frameDataPtr + 4;
	}else{
		return _frameDataPtr + 2;
	}
}

uint16_t NWResponse::getBodyLength(){
	if(_len > 255){
		return getPayloadLength() - 4;
	}else{
		return getPayloadLength() - 2;
	}
}

uint8_t NWResponse::getPayload(uint8_t index){
		return _frameDataPtr[index + 2];

}

uint8_t* NWResponse::getPayloadPtr(){

		return _frameDataPtr;

}

uint8_t NWResponse::getPayloadLength(){

	return _len;
}

#endif /* NETWORK_XXXXX */
