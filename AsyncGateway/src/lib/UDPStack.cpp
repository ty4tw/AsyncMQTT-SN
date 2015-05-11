/*
 * UDPStack.cpp
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

#ifdef NETWORK_UDP

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include "ProcessFramework.h"
#include "UDPStack.h"

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

void Network::unicast(const uint8_t* payload, uint16_t payloadLength, uint32_t msb, uint32_t ipAddress, uint16_t port){
	UDPPort::unicast(payload, payloadLength, ipAddress, port);
}

void Network::broadcast(const uint8_t* payload, uint16_t payloadLength){
	UDPPort::multicast(payload, payloadLength);
}

uint8_t* Network::getResponce(int* len){
	uint16_t recvLen = UDPPort::recv(_rxDataBuf, MQTTSN_MAX_FRAME_SIZE, &_ipAddress, &_portNo);
	if(recvLen < 0){
		*len = recvLen;
		return 0;
	}else{
		uint8_t pos = 0;
		if(_rxDataBuf[0] == 0x01){
			pos++;
		}
		if(recvLen != getUint16(_rxDataBuf + pos )){
			*len = 0;
			return 0;
		}else{
			*len = getUint16(_rxDataBuf + pos );
			return _rxDataBuf;
		}
	}
}

int Network::initialize(UdpConfig  config){
	return UDPPort::open(config);
}

uint32_t Network::getAddrMsb(void){
	return 0;
}

uint32_t Network::getAddrLsb(void){
	return _ipAddress;
}

uint16_t Network::getAddr16(void){
	return _portNo;
}

/*=========================================
       Class udpStack
 =========================================*/

UDPPort::UDPPort(){
    _disconReq = false;
    _sockfdUnicast = -1;
    _sockfdMulticast = -1;
	_gPortNo = 0;
	_gIpAddr = 0;

}

UDPPort::~UDPPort(){
    close();
}

void UDPPort::close(){
	if(_sockfdUnicast > 0){
		::close( _sockfdUnicast);
		_sockfdUnicast = -1;
	}
	if(_sockfdMulticast > 0){
		::close( _sockfdMulticast);
		_sockfdMulticast = -1;
	}
}

int UDPPort::open(UdpConfig config){
	char loopch = 0;
	const int reuse = 1;

	if(config.uPortNo == 0 || config.gPortNo == 0){
		return -1;
	}
	_gPortNo = htons(config.gPortNo);
	_gIpAddr = inet_addr(config.ipAddress);

	/*------ Create unicast socket --------*/
	_sockfdUnicast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_sockfdUnicast < 0){
		return -1;
	}

	setsockopt(_sockfdUnicast, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	sockaddr_in addru;
	addru.sin_family = AF_INET;
	addru.sin_port = htons(config.uPortNo);
	addru.sin_addr.s_addr = INADDR_ANY;

	if( ::bind ( _sockfdUnicast, (sockaddr*)&addru,  sizeof(addru)) <0){
		return -1;
	}
	if(setsockopt(_sockfdUnicast, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&loopch, sizeof(loopch)) <0 ){
		D_NWSTACK("error IP_MULTICAST_LOOP in UDPPort::open\n");
		close();
		return -1;
	}

	/*------ Create Multicast socket --------*/
	_sockfdMulticast = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_sockfdMulticast < 0){
		close();
		return -1;
	}

	setsockopt(_sockfdMulticast, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	sockaddr_in addrm;
	addrm.sin_family = AF_INET;
	addrm.sin_port = _gPortNo;
	addrm.sin_addr.s_addr = INADDR_ANY;

	if( ::bind ( _sockfdMulticast, (sockaddr*)&addrm,  sizeof(addrm)) <0){
		return -1;
	}
	if(setsockopt(_sockfdMulticast, IPPROTO_IP, IP_MULTICAST_LOOP,(char*)&loopch, sizeof(loopch)) <0 ){
		D_NWSTACK("error IP_MULTICAST_LOOP in UDPPort::open\n");
		close();
		return -1;
	}

	ip_mreq mreq;
	mreq.imr_interface.s_addr = INADDR_ANY;
	mreq.imr_multiaddr.s_addr = _gIpAddr;

	if( setsockopt(_sockfdMulticast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))< 0){
		D_NWSTACK("error Multicast IP_ADD_MEMBERSHIP in UDPPort::open\n");
		perror("multicast");
		close();
		return -1;
	}

	if( setsockopt(_sockfdUnicast, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq))< 0){
		D_NWSTACK("error Unicast IP_ADD_MEMBERSHIP in UDPPort::open\n");
		close();
		return -1;
	}
	return 0;
}


int UDPPort::unicast(const uint8_t* buf, uint32_t length, uint32_t ipAddress, uint16_t port  ){
	sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = port;
	dest.sin_addr.s_addr = ipAddress;

	int status = ::sendto( _sockfdUnicast, buf, length, 0, (const sockaddr*)&dest, sizeof(dest) );
	if( status < 0){
		D_NWSTACK("errno == %d in UDPPort::sendto\n", errno);
	}
	D_NWSTACK("sendto %s:%u length = %d\n",inet_ntoa(dest.sin_addr), htons(port), status);
	return status;
}

int UDPPort::multicast( const uint8_t* buf, uint32_t length ){
	return unicast(buf, length,_gIpAddr, _gPortNo);
}

int UDPPort::recv(uint8_t* buf, uint16_t len, uint32_t* ipAddressPtr, uint16_t* portPtr){
	fd_set recvfds;
	int maxSock = 0;

	FD_ZERO(&recvfds);
	FD_SET(_sockfdUnicast, &recvfds);
	FD_SET(_sockfdMulticast, &recvfds);

	if(_sockfdMulticast > _sockfdUnicast){
		maxSock = _sockfdMulticast;
	}else{
		maxSock = _sockfdUnicast;
	}

	select(maxSock + 1, &recvfds, 0, 0, 0);

	if(FD_ISSET(_sockfdUnicast, &recvfds)){
		return recvfrom (_sockfdUnicast,buf, len, 0,ipAddressPtr, portPtr );
	}else if(FD_ISSET(_sockfdMulticast, &recvfds)){
		return recvfrom (_sockfdMulticast,buf, len, 0,ipAddressPtr, portPtr );
	}
	return 0;
}

int UDPPort::recvfrom (int sockfd, uint8_t* buf, uint16_t len, uint8_t flags, uint32_t* ipAddressPtr, uint16_t* portPtr ){
	sockaddr_in sender;
	socklen_t addrlen = sizeof(sender);
	memset(&sender, 0, addrlen);

	int status = ::recvfrom( sockfd, buf, len, flags, (sockaddr*)&sender, &addrlen );

	if ( status < 0 && errno != EAGAIN )	{
		D_NWSTACK("errno == %d in UDPPort::recvfrom\n", errno);
		return -1;
	}
	*ipAddressPtr = (uint32_t)sender.sin_addr.s_addr;
	*portPtr = (uint16_t)sender.sin_port;
	D_NWSTACK("recved from %s:%d length = %d\n",inet_ntoa(sender.sin_addr),htons(*portPtr),status);
	return status;
}


#endif /* NETWORK_UDP */
