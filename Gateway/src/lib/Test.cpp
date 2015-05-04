/*
 * Test.cpp
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
 *      Author: tomoaki YAMAGUCHI
 *     Version: 0.0.1
 */
#include "Defines.h"
#include "Topics.h"
#include <stdio.h>
using namespace tomyAsyncGateway;

void test(){
	Topics topicTbl;
	Topic* tp;

	string topic1 = "topic/1/tomy";
	string topic2 = "topic/2/tomy";
	string topic3 = "topic/+";
	string topic4 = "topic/+/tomy";
	string topic5 = "topic/5/tomy";

	topicTbl.add(&topic1,1);
	topicTbl.add(&topic2,2);
	topicTbl.add(&topic3,3);
	topicTbl.add(&topic4,4);

	printf("TopicId %d\n",topicTbl.getTopicId(&topic1));
	printf("TopicId %d\n",topicTbl.getTopicId(&topic2));
	printf("TopicId %d\n",topicTbl.getTopicId(&topic3));
	printf("TopicId %d\n",topicTbl.getTopicId(&topic4));

	tp = topicTbl.match(&topic5);
	printf("Topic='%s'\n",tp->getTopicName()->c_str());
}



