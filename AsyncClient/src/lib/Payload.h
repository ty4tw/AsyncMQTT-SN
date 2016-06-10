/*
 * Payload.h
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

#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#ifdef ARDUINO
	#include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

#define MSGPACK_FALSE    0xc2
#define MSGPACK_TRUE     0xc3
#define MSGPACK_POSINT   0x80
#define MSGPACK_NEGINT   0xe0
#define MSGPACK_UINT8    0xcc
#define MSGPACK_UINT16   0xcd
#define MSGPACK_UINT32   0xce
#define MSGPACK_INT8     0xd0
#define MSGPACK_INT16    0xd1
#define MSGPACK_INT32    0xd2
#define MSGPACK_FLOAT32  0xca
#define MSGPACK_FIXSTR   0xa0
#define MSGPACK_STR8     0xd9
#define MSGPACK_STR16    0xda
#define MSGPACK_ARRAY15  0x90
#define MSGPACK_ARRAY16  0xdc
#define MSGPACK_MAX_ELEMENTS   50   // Less than 256

namespace tomyAsyncClient {
/*=====================================
        Class Payload
  =====================================*/
class Payload{
public:
	Payload();
	Payload(uint16_t len);
	~Payload();

/*---------------------------------------------
  getLen() and getRowData() are
  minimum required functions of Payload class.
----------------------------------------------*/
	uint16_t getLen();       // get data length
	uint8_t* getRowData();   // get data pointer

/*--- Functions for MessagePack ---*/
	void init(void);
	int8_t set_bool(bool val);
	int8_t set_uint32(uint32_t val);
	int8_t set_int32(int32_t val);
	int8_t set_float(float val);
	int8_t set_str(char* val);
	int8_t set_str(const char* val);
	int8_t set_array(uint8_t val);

	bool    get_bool(uint8_t index);
	uint8_t getArray(uint8_t index);
	uint32_t get_uint32(uint8_t index);
	int32_t  get_int32(uint8_t index);
    float    get_float(uint8_t index);
    const char* get_str(uint8_t index, uint16_t* len);

	void 	 getPayload(uint8_t* payload, uint16_t payloadLen);
	uint16_t getAvailableLength();
private:
	uint8_t* getBufferPos(uint8_t index);
	uint8_t* _buff;
	uint16_t _len;
	uint8_t  _elmCnt;
	uint8_t* _pos;
	uint8_t  _memDlt;
};

} /* tomyAsyncClient */

#endif /* PAYLOAD_H_ */
