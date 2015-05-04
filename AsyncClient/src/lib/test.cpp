/*
 * test.cpp
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
 *  Created on: 2015/04/15
 *    Modified: 
 *      Author: tomoaki
 *     Version: 0.0.1
 */

#include "MqttsnClientApp.h"
#include "MqttsnClient.h"
#include "GwProxy.h"
#include "RegisterManager.h"
#include "TopicTable.h"
#include <stdio.h>
#include <map>

using namespace std;
using namespace tomyAsyncClient;

typedef map<uint16_t, uint16_t> ACKMAP;

void test( ){

	/*
	printf("Test RegisterQue\n");
	RegisterManager que;

	const char* topic1 = "topic/1/tomy";
	const char* topic2 = "topic/2/tomy";
	const char* topic3 = "topic/+";
	const char* topic4 = "topic/+/tomy";
	const char* topic5 = "topic/5/tomy";

	//que.add(topic1, (uint16_t)1);
	//que.add(topic2, (uint16_t)2);
	//que.add(topic3, (uint16_t)3);
	//que.add(topic4, (uint16_t)4);

	printf("Topic %s\n",que.getTopic(1));
	printf("Topic %s\n",que.getTopic(2));
	printf("Topic %s\n",que.getTopic(3));
	printf("Topic %s\n",que.getTopic(4));
	printf("Topic %s\n",que.getTopic(5));

	//que.remove(3);
	printf("Topic %s\n",que.getTopic(1));
	printf("Topic %s\n",que.getTopic(2));
	printf("Topic %s\n",que.getTopic(3));
	printf("Topic %s\n",que.getTopic(4));
	printf("Topic %s\n",que.getTopic(5));

	printf("Test TopicTable\n");

	TopicTable topicTbl;
	Topic* tp;

	topicTbl.add(topic1,1);
	topicTbl.add(topic2,2);
	topicTbl.add(topic3,3);
	topicTbl.add(topic4,4);

	printf("TopicId %d\n",topicTbl.getTopicId(topic1));
	printf("TopicId %d\n",topicTbl.getTopicId(topic2));
	printf("TopicId %d\n",topicTbl.getTopicId(topic3));
	printf("TopicId %d\n",topicTbl.getTopicId(topic4));

	tp = topicTbl.match(topic5);
	printf("Topic='%s'\n",topicTbl.getTopicName(tp));
	*/


}
