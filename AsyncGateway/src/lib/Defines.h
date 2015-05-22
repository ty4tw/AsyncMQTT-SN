/*  Defines.h
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
 *    Modified: 2015/05/16
 *      Author: Tomoaki YAMAGUCHI
 *     Version: 0.1.0
 */
#ifndef  DEFINES_H_
#define  DEFINES_H_

/*=================================
 *    Network  Selection
 =================================*/
#if ! defined(NETWORK_UDP) && ! defined (NETWORK_XXXXX)
#define NETWORK_XBEE
#define GATEWAY_NETWORK  "Network is XBee."
#endif

#ifdef NETWORK_UDP
#define GATEWAY_NETWORK  "Network is UDP."
#endif

#ifdef NETWORK_XXXXX
#define GATEWAY_NETWORK  "Network is XXXXX."
#endif

/*=================================
 *    CPU TYPE
 ==================================*/
#define CPU_LITTLEENDIANN
//#define CPU_BIGENDIANN

/*=================================
 *    Debug LOG
 ==================================*/
#define DEBUG_NWSTACK

/*=================================
      Debug Print functions
 ==================================*/
#ifdef  DEBUG_NWSTACK
  #define D_NWSTACK(...) printf(__VA_ARGS__)
#else
  #define D_NWSTACK(...)
#endif

/*=================================
 *    Data Type
 ==================================*/
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;


typedef struct {
	long baudrate;
	char* device;
	unsigned int flag;
}XBeeConfig;

typedef struct {
	char* ipAddress;
	uint16_t gPortNo;
	uint16_t uPortNo;
}UdpConfig;

typedef struct {
	uint8_t  param1;
	uint16_t param2;
	uint16_t param3;
}XXXXXConfig;

#ifdef NETWORK_XBEE
#define NETWORK_CONFIG  XBeeConfig
#endif

#ifdef NETWORK_UDP
#define NETWORK_CONFIG UdpConfig
#endif

#ifdef NETWORK_XXXXX
#define NETWORK_CONFIG XXXXXConfig
#endif

#endif  /*  DEFINES_H_  */
