/*
 * ClientSendTask.h
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
#ifndef CLIENTSENDTASK_H_
#define CLIENTSENDTASK_H_

#include "lib/ProcessFramework.h"
#include "GatewayResourcesProvider.h"

using namespace tomyAsyncGateway;
class ClientSendTask:public Thread{
	MAGIC_WORD_FOR_TASK;
public:
	ClientSendTask(GatewayResourcesProvider* res);
	~ClientSendTask();

	void run();

private:
	GatewayResourcesProvider* _res;
	Network* _network;
};

#endif /* CLIENTSENDTASK_H_ */


