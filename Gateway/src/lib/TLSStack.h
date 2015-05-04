/*
 * TLSStack.h
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

#ifndef TLSSTACK_H
#define TLSSTACK_H

#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include "Defines.h"
#include "Messages.h"
#include "TCPStack.h"

#define TLS_CA_DIR      "/etc/ssl/certs"

using namespace std;
namespace tomyAsyncGateway{
/*========================================
       Class TLSStack
 =======================================*/
class TLSStack:public TCPStack{
public:
	TLSStack();
	TLSStack(bool secure);
	virtual ~TLSStack();

	bool connect( const char* host, const char* service );
	void disconnect();
	int send( const uint8_t* buf, uint16_t length );
	int  recv ( uint8_t* buf, uint16_t len );

	bool isValid();
	bool isSecure();
	int  getSock();
	SSL* getSSL();
private:
	static SSL_CTX* _ctx;
	static int  _numOfInstance;
	static SSL_SESSION* _session;

	SSL*     _ssl;
	bool   _secureFlg;
	bool   _disconReq;
	Mutex _mutex;
	bool  _busy;
};

}

#endif /* TLSSTACK_H */
