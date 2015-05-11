/*
 * UDPStack.h
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

#ifndef UDPSTACK_H_
#define UDPSTACK_H_

#include "Defines.h"
#ifdef NETWORK_UDP

#include "ProcessFramework.h"
#include <sys/time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

/*
 *   MQTTS  Client's state
 */
#define MQTTS_DEVICE_DISCONNECTED     0
#define MQTTS_DEVICE_ACTIVE           1
#define MQTTS_DEVICE_ASLEEP           2
#define MQTTS_DEVICE_AWAKE            3
#define MQTTS_DEVICE_LOST             4

#define MQTTSN_MAX_FRAME_SIZE      1024

using namespace std;

namespace tomyAsyncGateway{

/*========================================
       Class UpdPort
 =======================================*/
class UDPPort{
public:
	UDPPort();
	virtual ~UDPPort();

	int open(UdpConfig config);

	int unicast(const uint8_t* buf, uint32_t length, uint32_t ipaddress, uint16_t port  );
	int multicast( const uint8_t* buf, uint32_t length );
	int recv(uint8_t* buf, uint16_t len, uint32_t* ipaddress, uint16_t* port );

private:
	void close();
	void setNonBlocking( const bool );
	int recvfrom (int sockfd, uint8_t* buf, uint16_t len, uint8_t flags, uint32_t* ipaddress, uint16_t* port );

	int _sockfdUnicast;
	int _sockfdMulticast;

	uint16_t _gPortNo;
	uint32_t _gIpAddr;
	bool    _disconReq;

};

/*===========================================
               Class  Network
 ============================================*/
class Network:public UDPPort{
public:
    Network();
    ~Network();

    void unicast(const uint8_t* payload, uint16_t payloadLength, uint32_t msb, uint32_t ipAddress, uint16_t port);
	void broadcast(const uint8_t* payload, uint16_t payloadLength);
	uint8_t* getResponce(int* len);
    int  initialize(NETWORK_CONFIG  config);
    uint32_t getAddrMsb(void);
    uint32_t getAddrLsb(void);
    uint16_t getAddr16(void);

private:
    uint32_t _ipAddress;
    uint16_t _portNo;
    uint8_t  _rxDataBuf[MQTTSN_MAX_FRAME_SIZE + 1];
};


}    /* end of namespace */

#endif /* NETWORK_UDP */
#endif  /* UDPSTACK_H_ */
