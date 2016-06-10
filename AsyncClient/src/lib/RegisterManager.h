/*
 * RegisterQue.h
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

#ifndef REGISTERQUE_H_
#define REGISTERQUE_H_

#ifdef ARDUINO
	#include <MqttsnClientApp.h>
#else
	#include "MqttsnClientApp.h"
#endif

namespace tomyAsyncClient {
/*======================================
      structure RegisterQue
 ======================================*/
typedef struct RegQueElement{
	const char* topicName;
	uint16_t msgId;
    int      retryCount;
    uint32_t sendUTC;
	RegQueElement* prev;
	RegQueElement* next;
}RegQueElement;

class RegisterManager{
public:
	RegisterManager();
	~RegisterManager();
	void registerTopic(char* topicName);
	void responceRegAck(uint16_t msgId, uint16_t topicId);
	void responceRegister(uint8_t* msg, uint16_t msglen);
	bool isDone(void);
	uint8_t checkTimeout();
	const char* getTopic(uint16_t msgId);
private:
	RegQueElement* getElement(const char* topicName);
	RegQueElement* getElement(uint16_t msgId);
	RegQueElement* add(const char* topicName, uint16_t msgId);
	void remove(RegQueElement* elm);
	void send(RegQueElement* elm);
	RegQueElement* _first;
};
}

#endif /* REGISTERQUE_H_ */
