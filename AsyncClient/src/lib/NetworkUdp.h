/*
 * NetworkUDP.h
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

#ifndef NETWORKUDP_H_
#define NETWORKUDP_H_


#ifdef ARDUINO
	#include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#ifdef NETWORK_UDP

#ifdef ARDUINO
	#include <SPI.h>
	#include <Ethernet.h>
	#include <EthernetUdp.h>
    #if ARDUINO >= 100
        #include "Arduino.h"
        #include <inttypes.h>
    #else
        #if ARDUINO < 100
            #include "WProgram.h"
            #include <inttypes.h>
        #endif
    #endif
#endif

#ifdef LINUX
    #include <sys/time.h>
    #include <iostream>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <string>
	#include <arpa/inet.h>
#endif

#define SOCKET_MAXHOSTNAME  200
#define SOCKET_MAXCONNECTIONS  5
#define SOCKET_MAXRECV  500
#define SOCKET_MAXBUFFER_LENGTH 500 // buffer size

//#define PACKET_TIMEOUT_CHECK   200  // msec

#define STAT_UNICAST   1
#define STAT_MULTICAST 2

using namespace std;

namespace tomyAsyncClient {
/*========================================
       Class UpdPort
 =======================================*/
class UdpPort{
    friend class Network;
public:
	UdpPort();
	virtual ~UdpPort();

	bool open(UdpConfig config);

	int unicast(const uint8_t* buf, uint32_t length, uint32_t ipaddress, uint16_t port  );
	int multicast( const uint8_t* buf, uint32_t length );
	int recv(uint8_t* buf, uint16_t len, bool nonblock, uint32_t* ipaddress, uint16_t* port );
	int recv(uint8_t* buf, int flags);
	bool checkRecvBuf();
	bool isUnicast();

private:
	void close();
	int recvfrom ( uint8_t* buf, uint16_t len, int flags, uint32_t* ipaddress, uint16_t* port );

#ifdef LINUX
	int      _sockfdUcast;
	int      _sockfdMcast;
	uint16_t _gPortNo;
	uint16_t _uPortNo;
	uint32_t _gIpAddr;
	uint8_t  _castStat;
#endif
#ifdef ARDUINO
	EthernetUDP _udpUnicast;
	EthernetUDP _udpMulticast;
	IPAddress   _gIpAddr;
	IPAddress   _cIpAddr;
	uint16_t    _gPortNo;
	uint16_t    _uPortNo;
	uint8_t*    _macAddr;
	uint8_t     _castStat;
#endif

	bool   _disconReq;

};

#define NO_ERROR	0
#define PACKET_EXCEEDS_LENGTH  1
/*===========================================
               Class  Network
 ============================================*/
class Network : public UdpPort {
public:
    Network();
    ~Network();

    int  broadcast(const uint8_t* payload, uint16_t payloadLen);
    int  unicast(const uint8_t* payload, uint16_t payloadLen);
    void setGwAddress(void);
    void resetGwAddress(void);
    void setFixedGwAddress(void);
    bool initialize(NETWORK_CONFIG  config);
    uint8_t*  getMessage(int* len);
private:
    void setSleep();
    int  readApiFrame(void);

    uint32_t _gwIpAddress;
	uint16_t _gwPortNo;
	uint32_t _ipAddress;
	uint16_t _portNo;
    int     _returnCode;
    bool _sleepflg;
    uint8_t _rxDataBuf[MQTTSN_MAX_PACKET_SIZE + 1];  // defined in MqttsnClientApp.h

};


}    /* end of namespace */

#endif /* NETWORK_UDP */
#endif /* NETWORKUDP_H_ */
