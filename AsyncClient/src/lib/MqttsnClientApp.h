/*
 * MqttsnClientApp.h
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

#ifndef MQTTSNCLIENTAPP_H_
#define MQTTSNCLIENTAPP_H_

/****************************************
      Select Network
*****************************************/

/*-------- Select Network  -------------*/
//#define NETWORK_UDP


#if ! defined(NETWORK_UDP) && ! defined (NETWORK_XXXXX)
#define NETWORK_XBEE
#endif

#ifdef NETWORK_UDP
#define BROADCAST_ENABLE
#endif


/*--- XBee Buffer Flow Control --*/

//#define XBEE_FLOWCTL_CRTSCTS

/*======================================
 *      LED PIN No of Arduino
 ======================================*/
#define ARDUINO_LED_PIN   13

/*======================================
 *         Debug Flag
 ======================================*/
#define DEBUG_NW
#define DEBUG_MQTTSN

/****************************************
      MQTT-SN Parameters
*****************************************/
#define MAX_INFLIGHT_MSG   10

#if defined(NETWORK_XBEE) || defined(ARDUINO)
    #define MQTTSN_MAX_MSG_LENGTH    70
    #define MQTTSN_MAX_PACKET_SIZE  128
#else
	#define MQTTSN_MAX_MSG_LENGTH  1024
    #define MQTTSN_MAX_PACKET_SIZE 1024
#endif

#define MQTTSN_DEFAULT_KEEPALIVE   900     // 1H
#define MQTTSN_DEFAULT_DURATION    900     // 15min
#define MQTTSN_TIME_SEARCHGW         3
#define MQTTSN_TIME_RETRY           10
#define MQTTSN_RETRY_COUNT           3

/****************************************
            Platform
*****************************************/
#ifndef ARDUINO
	#define LINUX
  //#define CPU_BIGENDIANN
#endif

/****************************************
      Application config structures
*****************************************/
#ifdef LINUX
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed int     int32_t;
#endif


#ifdef ARDUINO
	#include <Arduino.h>
#endif

struct MqttsnConfig{
	uint16_t keepAlive;
	bool     cleanSession;
	bool     sleep;
	const char* willTopic;
	const char* willMsg;
    uint8_t  willQos;
    bool     willRetain;
};

struct XBeeConfig{
	const char* clientId;
	long baudrate;
	uint8_t  portNo;
	const char* device;
};

struct XBeeAppConfig{
	XBeeConfig netCfg;
	MqttsnConfig mqttsnCfg;
};

struct UdpConfig{
	const char* clientId;
	uint8_t  ipAddress[4];
	uint16_t gPortNo;
	uint8_t  ipLocal[4];
	uint16_t uPortNo;
	uint8_t  macAddr[6];
};

struct UdpAppConfig{
	UdpConfig netCfg;
	MqttsnConfig mqttsnCfg;
};

/*======================================
      MACROs for Application
=======================================*/
#ifdef NETWORK_XBEE
    #define XBEE_APP_CONFIG       XBeeAppConfig  theAppConfig
	#define APP_CONFIG            XBeeAppConfig
	#define NETWORK_CONFIG        XBeeConfig
#endif

#ifdef NETWORK_UDP
    #define UDP_APP_CONFIG        UdpAppConfig   theAppConfig
	#define APP_CONFIG            UdpAppConfig
	#define NETWORK_CONFIG        UdpConfig
#endif

#define PUBLISH(...)     theClient->publish(__VA_ARGS__)
#define SUBSCRIBE(...)   theClient->subscribe(__VA_ARGS__)
#define UNSUBSCRIBE(...) theClient->unsubscribe(__VA_ARGS__)
#define DISCONNECT(...)  theClient->disconnect(__VA_ARGS__)

#define TASK_LIST         TaskList theTaskList[]
#define TASK(...)         {__VA_ARGS__, 0, 0}
#define END_OF_TASK_LIST  {0,0, 0, 0}
#define SUBSCRIBE_LIST    OnPublishList theOnPublishList[]
#define SUB(...)          {__VA_ARGS__}
#define END_OF_SUBSCRIBE_LIST {0,0,0}

#define INDICATOR_ON(...)   theClient->indicator(__VA_ARGS__)
/*======================================
      MACROs for debugging
========================================*/
#ifndef DEBUG_NW
	#define D_NWA(...)
    #define D_NWALN(...)
	#define D_NWL(...)
#endif

#ifndef DEBUG_MQTTSN
	#define D_MQTTA(...)
	#define D_MQTTALN(...)
	#define D_MQTTL(...)
#endif

#if defined(DEBUG_NW) && defined(ARDUINO)
	#define D_NWA(...)    debug.print(__VA_ARGS__)
	#define D_NWALN(...)  debug.println(__VA_ARGS__)
	#define D_NWL(...)
#endif

#if defined(DEBUG_NW) && ! defined(ARDUINO)
	#define D_NWA(...)
	#define D_NWALN(...)
	#define D_NWL(...)    printf(__VA_ARGS__)
#endif

#if defined(DEBUG_MQTTSN) && defined(ARDUINO)
    #define D_MQTTA(...)  debug.print(__VA_ARGS__)
	#define D_MQTTALN(...)  debug.println(__VA_ARGS__)
	#define D_MQTTL(...)
#endif

#if defined(DEBUG_MQTTSN) && ! defined(ARDUINO)
	#define D_MQTTA(...)
	#define D_MQTTALN(...)
    #define D_MQTTL(...)  printf(__VA_ARGS__)
#endif

/*======================================
      MQTT-SN Defines
========================================*/
#define MQTTSN_TYPE_ADVERTISE     0x00
#define MQTTSN_TYPE_SEARCHGW      0x01
#define MQTTSN_TYPE_GWINFO        0x02
#define MQTTSN_TYPE_CONNECT       0x04
#define MQTTSN_TYPE_CONNACK       0x05
#define MQTTSN_TYPE_WILLTOPICREQ  0x06
#define MQTTSN_TYPE_WILLTOPIC     0x07
#define MQTTSN_TYPE_WILLMSGREQ    0x08
#define MQTTSN_TYPE_WILLMSG       0x09
#define MQTTSN_TYPE_REGISTER      0x0A
#define MQTTSN_TYPE_REGACK        0x0B
#define MQTTSN_TYPE_PUBLISH       0x0C
#define MQTTSN_TYPE_PUBACK        0x0D
#define MQTTSN_TYPE_PUBCOMP       0x0E
#define MQTTSN_TYPE_PUBREC        0x0F
#define MQTTSN_TYPE_PUBREL        0x10
#define MQTTSN_TYPE_SUBSCRIBE     0x12
#define MQTTSN_TYPE_SUBACK        0x13
#define MQTTSN_TYPE_UNSUBSCRIBE   0x14
#define MQTTSN_TYPE_UNSUBACK      0x15
#define MQTTSN_TYPE_PINGREQ       0x16
#define MQTTSN_TYPE_PINGRESP      0x17
#define MQTTSN_TYPE_DISCONNECT    0x18
#define MQTTSN_TYPE_WILLTOPICUPD  0x1A
#define MQTTSN_TYPE_WILLTOPICRESP 0x1B
#define MQTTSN_TYPE_WILLMSGUPD    0x1C
#define MQTTSN_TYPE_WILLMSGRESP   0x1D

#define MQTTSN_TOPIC_TYPE_NORMAL     0x00
#define MQTTSN_TOPIC_TYPE_PREDEFINED 0x01
#define MQTTSN_TOPIC_TYPE_SHORT      0x02
#define MQTTSN_TOPIC_TYPE            0x03

#define MQTTSN_FLAG_DUP     0x80
#define MQTTSN_FLAG_QOS_0   0x0
#define MQTTSN_FLAG_QOS_1   0x20
#define MQTTSN_FLAG_QOS_2   0x40
#define MQTTSN_FLAG_QOS_N1  0xc0
#define MQTTSN_FLAG_RETAIN  0x10
#define MQTTSN_FLAG_WILL    0x08
#define MQTTSN_FLAG_CLEAN   0x04

#define MQTTSN_PROTOCOL_ID  0x01
#define MQTTSN_HEADER_SIZE  2

#define MQTTSN_RC_ACCEPTED                  0x00
#define MQTTSN_RC_REJECTED_CONGESTION       0x01
#define MQTTSN_RC_REJECTED_INVALID_TOPIC_ID 0x02
#define MQTTSN_RC_REJECTED_NOT_SUPPORTED    0x03

#define MQTTSN_TOPICID_PREDEFINED_TIME      0x01

#endif /* MQTTSNCLIENTAPP_H_ */
