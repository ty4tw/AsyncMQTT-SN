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

#ifndef XXXXXSTACK_H_
#define XXXXXSTACK_H_

#include "Defines.h"
#ifdef NETWORK_XXXXX

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
/*============================================
              NWAddress64
 =============================================*/
class NWAddress64 {
public:
	NWAddress64(uint32_t msb, uint32_t lsb);
	NWAddress64(void);
	uint32_t getMsb();
	uint32_t getLsb();
	void setMsb(uint32_t msb);
	void setLsb(uint32_t lsb);
	bool operator==(NWAddress64&);
private:
	uint32_t _msb;
	uint32_t _lsb;
};

/*============================================
               NWResponse
 =============================================*/

class NWResponse {
public:
	NWResponse();
	uint8_t  getMsgType();
	uint8_t  getFrameLength();
	uint8_t  getPayload(uint8_t index);
	uint8_t* getPayloadPtr();
	uint8_t* getBody();
	uint16_t getBodyLength();
	uint8_t  getPayloadLength();
	uint16_t getClientAddress16();
	NWAddress64* getClientAddress64();

	void setLength(uint16_t len);
  	void setMsgType(uint8_t type);
	void setClientAddress64(uint32_t msb, uint32_t ipAddress);
	void setClientAddress16(uint16_t portNo);
private:
	NWAddress64 _addr64;
	uint16_t _addr16;
	uint16_t _len;
	uint8_t  _type;
	uint8_t _frameDataPtr[MQTTSN_MAX_FRAME_SIZE];
};


/*========================================
       Class XXXXXPort
 =======================================*/
class XXXXXPort{
public:
	XXXXXPort();
	virtual ~XXXXXPort();

	int initialize(XXXXXConfig config);
    int initialize();

	int unicast(  );
	int multicast(  );
	int recv( );

private:
	void close();

	XXXXXConfig _config;

};

/*===========================================
               Class  Network
 ============================================*/
class Network:public XXXXXPort{
public:
    Network();
    ~Network();

    void unicast(NWAddress64* addr64, uint16_t addr16,	uint8_t* payload, uint16_t payloadLength);
	void broadcast(uint8_t* payload, uint16_t payloadLength);
	bool getResponse(NWResponse* response);
    int  initialize(XXXXXConfig  config);

private:

};


}    /* end of namespace */

#endif /* NETWORK_UDP */
#endif  /* UDPSTACK_H_ */
