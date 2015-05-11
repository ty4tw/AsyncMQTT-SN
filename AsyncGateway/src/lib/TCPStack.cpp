/*
 * Socket.cpp
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

#include "Defines.h"
#include "ProcessFramework.h"
#include "TCPStack.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

using namespace std;
using namespace tomyAsyncGateway;
extern char* currentDateTime();

/*========================================
       Class TCPStack
 =======================================*/
TCPStack::TCPStack(){
    _addrinfo = 0;
    _disconReq = false;
    _sockfd = -1;
}

TCPStack::~TCPStack(){
    if(_addrinfo){
		freeaddrinfo(_addrinfo);
	}
}

bool TCPStack::isValid(){
	if(_sockfd > 0){
		if(_disconReq){
			close();
			_sem.post();
		}else{
			return true;
		}
	}
	return false;
}

void TCPStack::disconnect(){
    if ( _sockfd > 0 ){
    	_disconReq = true;
    	_sem.wait();
    }
}

void TCPStack::close(){
	if(_sockfd > 0){
		::close(_sockfd);
		_sockfd = -1;
		_disconReq = false;
		if(_addrinfo){
			freeaddrinfo(_addrinfo);
			_addrinfo = 0;
		}
	}
}

bool TCPStack::bind ( const char* service ){
	if(isValid()){
		return false;
	}
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

	if (_addrinfo){
		freeaddrinfo(_addrinfo);
	}
	int err = getaddrinfo(0, service, &hints, &_addrinfo);
    if (err) {
    	LOGWRITE("\n%s   \x1b[0m\x1b[31merror:\x1b[0m\x1b[37mgetaddrinfo(): %s\n", currentDateTime(),gai_strerror(err));
        return false;
    }

	_sockfd = socket(_addrinfo->ai_family, _addrinfo->ai_socktype, _addrinfo->ai_protocol);
    if (_sockfd < 0){
    	return false;
	}
	int on = 1;
	if ( setsockopt ( _sockfd, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 ){
		return false;
	}

	if( ::bind ( _sockfd, _addrinfo->ai_addr,  _addrinfo->ai_addrlen ) <0){
	    return false;
	}
	return true;
}

bool TCPStack::listen() {
	if ( !isValid() ){
	    return false;
	}
	int listen_return = ::listen ( _sockfd, SOCKET_MAXCONNECTIONS );
	if ( listen_return == -1 ){
	    return false;
	}
	return true;
}


bool TCPStack::accept ( TCPStack& new_socket ){
	sockaddr_storage sa;
	socklen_t len = sizeof ( sa );
	new_socket._sockfd = ::accept ( _sockfd, (struct sockaddr*) &sa,  &len );
	if ( new_socket._sockfd <= 0 ){
		return false;
	}else{
		return true;
	}
}

int TCPStack::send (const uint8_t* buf, uint16_t length  ){
	return ::send ( _sockfd, buf, length, MSG_NOSIGNAL );
}

int TCPStack::recv ( uint8_t* buf, uint16_t len ){
	return ::recv ( _sockfd, buf, len, 0 );
}


bool TCPStack::connect ( const char* host, const char* service ){
	if(isValid()){
		return false;
	}
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if (_addrinfo){
		freeaddrinfo(_addrinfo);
	}

	int err = getaddrinfo(host, service, &hints, &_addrinfo);
    if (err) {
    	LOGWRITE("\n%s   \x1b[0m\x1b[31merror:\x1b[0m\x1b[37mgetaddrinfo(): %s\n", currentDateTime(),gai_strerror(err));
        return false;
    }

	int sockfd = socket(_addrinfo->ai_family, _addrinfo->ai_socktype, _addrinfo->ai_protocol);

    if (sockfd < 0){
    	return false;
	}
	int on = 1;

	if ( setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 ){
		return false;
	}

	if( ::connect (sockfd, _addrinfo->ai_addr,  _addrinfo->ai_addrlen ) <0){
		//perror("TCPStack connect");
		::close(sockfd);
	    return false;
	}

	_sockfd = sockfd;
	return true;
}

void TCPStack::setNonBlocking ( const bool b ){
	int opts;

	opts = fcntl ( _sockfd, F_GETFL );

	if ( opts < 0 ){
	    return;
	}

	if ( b ){
	    opts = ( opts | O_NONBLOCK );
	}else{
	    opts = ( opts & ~O_NONBLOCK );
	}
	fcntl ( _sockfd,  F_SETFL,opts );
}
