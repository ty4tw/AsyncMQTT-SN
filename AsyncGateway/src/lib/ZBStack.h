/*
 * ZBStack.h
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

#ifndef ZBSTACK_H_
#define ZBSTACK_H_

#include "Defines.h"
#ifdef NETWORK_XBEE

#include <sys/time.h>
#include <iostream>
#include "ProcessFramework.h"
#include <termios.h>

#define START_BYTE 0x7e
#define ESCAPE     0x7d
#define XON        0x11
#define XOFF       0x13

#define MAX_FRAME_DATA_SIZE  128

#define DEFAULT_FRAME_ID 1

#define XB_PACKET_ACKNOWLEGED  0x01
#define XB_BROADCAST_PACKET    0x02
#define XB_BROADCAST_RADIUS_MAX_HOPS 0

#define ZB_API_ID_INDEX  3
//#define PACKET_OVERHEAD_LENGTH 6
//#define ZB_TX_API_LENGTH  12
//#define ZB_PAYLOAD_OFFSET 11
#define ZB_RX_PAYLOAD_OFFSET 15
/**
 * API ID Constant
 */
#define XB_API_REQUEST               0x10
#define XB_API_RESPONSE              0x90
#define XB_API_MODEMSTATUS           0x8A

//#define NW_TX_UNICAST 0
//#define NW_TX_BROADCAST 8

#define NW_MAX_NODEID 24

/*
 *   MQTTS  Client's state
 */
#define MQTTS_DEVICE_DISCONNECTED     0
#define MQTTS_DEVICE_ACTIVE           1
#define MQTTS_DEVICE_ASLEEP           2
#define MQTTS_DEVICE_AWAKE            3
#define MQTTS_DEVICE_LOST             4

#define XB_PACKET_ACKNOWLEGED        0x01
#define XB_BROADCAST_PACKET          0x02
#define XB_BROADCAST_RADIUS_MAX_HOPS 0

#define ZB_API_ID_POS                 3
#define PACKET_OVERHEAD_LENGTH        6

#define PACKET_TIMEOUT_CHECK        200   // 100ms

/*====  STATUS ====== */
#define NO_ERROR               0
#define CHECKSUM_ERROR         1
#define PACKET_OVERFLOW        2
#define UNEXPECTED_START_BYTE  3

using namespace std;
namespace tomyAsyncGateway{

/*===========================================
                SerialPort
 ============================================*/
class SerialPort{
public:
	SerialPort();
	~SerialPort();
	int open(XBeeConfig  config);
	bool send(unsigned char b);
	bool recv(unsigned char* b);
	void flush();

private:
	int open(const char* devName, unsigned int boaurate,  bool parity, unsigned int stopbit, unsigned int flg);

	int _fd;  // file descriptor
	struct termios _tio;
};

/*===========================================
               Class  NetworkStack
 ============================================*/
class Network{
public:
	Network();
	~Network();
	int  unicast(const uint8_t* payload, uint16_t payloadLen, uint32_t addrMsb, uint32_t addrLsb, uint16_t addr16);
	int  broadcast(const uint8_t* payload, uint16_t payloadLen);
	int  initialize(XBeeConfig  config);
	uint8_t* getResponce(int* len);
	uint32_t getAddrMsb(void);
	uint32_t getAddrLsb(void);
	uint16_t getAddr16(void);
private:
	void setSerialPort(SerialPort *serialPort);
    void send(const uint8_t* payload, uint8_t payloadLen, uint32_t msb = 0x00000000, uint32_t lsb = 0x0000ffff, uint16_t addr16 = 0xfffe);
	bool readApiFrame(uint16_t timeoutMillsec);
    void readApiFrame(void);
    bool read(uint8_t* buff);
    void write(uint8_t val);
    void sendByte(uint8_t);
    void sendAddr(uint8_t* addr, uint8_t len, uint8_t* checksum);

    uint8_t     _responseData[MAX_FRAME_DATA_SIZE + 1];
    uint8_t*    _mqttsnMsg;
    int        _respLen;
    Timer       _tm;
    SerialPort* _serialPort;
    uint8_t     _pos;
    uint8_t     _byteData;
    uint8_t     _checksumTotal;
    uint8_t     _errorCode;
    uint8_t     _available;
    int  _returnCode;
    bool _esc;
    bool _sleepflg;
};

}

#endif /* NETWORK_XBEE */
#endif  /* ZBSTACK_H_ */
