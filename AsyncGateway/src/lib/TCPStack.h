/*
 * TCPStack.h
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

#ifndef TCPSTACK_H
#define TCPSTACK_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include "Defines.h"
#include "Messages.h"

#define SOCKET_MAXCONNECTIONS  5
#define SOCKET_MAXBUFFER_LENGTH 1280 // buffer size

using namespace std;
namespace tomyAsyncGateway{
/*========================================
       Class TCPStack
 =======================================*/
class TCPStack{
public:
	TCPStack();
	virtual ~TCPStack();
	void disconnect();

	// Server initialization
	bool bind( const char* service );
	bool listen();
	bool accept( TCPStack& );

	// Client initialization
	bool connect( const char* host, const char* service );

	int send( const uint8_t* buf, uint16_t length );
	int  recv ( uint8_t* buf, uint16_t len );
	void close();

	void setNonBlocking( const bool );

	bool isValid();
	int getSock(){return _sockfd;}
private:
	int _sockfd;
	addrinfo* _addrinfo;
	Semaphore _sem;
	bool   _disconReq;

};
}


#endif /* TCPSTACK_H */
