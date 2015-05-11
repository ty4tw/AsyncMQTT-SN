/*
 * TLSStack.cpp
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
#include "TLSStack.h"
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

using namespace std;
using namespace tomyAsyncGateway;

int TLSStack::_numOfInstance = 0;
SSL_CTX* TLSStack::_ctx = 0;
SSL_SESSION* TLSStack::_session = 0;

/*========================================
       Class TLSStack
 =======================================*/
TLSStack::TLSStack(){
    TLSStack(true);
}

TLSStack::TLSStack(bool secure):TCPStack(){
	char error[256];
	if(secure){
	    _numOfInstance++;
		if(_ctx == 0){
			SSL_load_error_strings();
			SSL_library_init();
			_ctx = SSL_CTX_new(TLSv1_2_client_method());
			if(_ctx == 0){
				ERR_error_string_n(ERR_get_error(), error, sizeof(error));
				LOGWRITE("SSL_CTX_new() %s\n",error);
				THROW_EXCEPTION(ExFatal, ERRNO_SYS_01, "can't create SSL context.");
			}
			if(!SSL_CTX_load_verify_locations(_ctx, 0,TLS_CA_DIR)){
				ERR_error_string_n(ERR_get_error(), error, sizeof(error));
				LOGWRITE("SSL_CTX_load_verify_locations() %s\n",error);
				THROW_EXCEPTION(ExFatal, ERRNO_SYS_01, "can't load CA_LIST.");
			}
		}
	}
    _ssl = 0;
    _disconReq = false;
    _secureFlg = secure;
    _busy = false;
}

TLSStack::~TLSStack(){
	if(_secureFlg){
		_numOfInstance--;
	}
    if(_ssl){
		SSL_free(_ssl);
	}
    if(_session && _numOfInstance == 0){
    	SSL_SESSION_free(_session);
    	_session = 0;
    }
    if(_ctx && _numOfInstance == 0){
    	SSL_CTX_free(_ctx);
    	_ctx = 0;
        ERR_free_strings();
    }
}

bool TLSStack::connect ( const char* host, const char* service ){
	char errmsg[256];
	int rc = 0;
	char peer_CN[256];
	SSL_SESSION* sess = 0;
	X509* peer;

	if(isValid()){
		return false;
	}
	if(!TCPStack::connect(host, service)){
		return false;
	}
	if(!_secureFlg){
		return true;
	}

	SSL* ssl = SSL_new(_ctx);
	if(ssl == 0){
		ERR_error_string_n(ERR_get_error(), errmsg, sizeof(errmsg));
		LOGWRITE("SSL_new()  %s\n",errmsg);
		return false;
	}

	rc = SSL_set_fd(ssl, TCPStack::getSock());
	if(rc == 0){
		SSL_free(ssl);
		return false;
	}

	if(_session){
		rc = SSL_set_session(ssl, sess);
	}else{
		rc = SSL_connect(ssl);
	}
	if(rc != 1){
		ERR_error_string_n(ERR_get_error(), errmsg, sizeof(errmsg));
		LOGWRITE("SSL_connect() %s\n",errmsg);
		SSL_free(ssl);
		return false;
	}

	if(SSL_get_verify_result(ssl) != X509_V_OK){
		LOGWRITE("SSL_get_verify_result() error: Certificate doesn't verify.\n");
		SSL_free(ssl);
		return false;
	}

	peer = SSL_get_peer_certificate(ssl);
	X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, peer_CN, 256);
	if(strcasecmp(peer_CN, host)){
		LOGWRITE("SSL_get_peer_certificate() error: Broker dosen't much host name.\n");
		SSL_free(ssl);
		return false;
	}
	if(_session == 0){
		_session = sess;
	}
	_ssl = ssl;
	return true;
}


int TLSStack::send (const uint8_t* buf, uint16_t length  ){
	char errmsg[256];
	fd_set rset;
	fd_set wset;
	bool writeBlockedOnRead = false;
	int bpos = 0;

	if(_secureFlg){
		_mutex.lock();
		_busy = true;

		while(true){
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			FD_SET(getSock(), &rset);
			FD_SET(getSock(), &wset);

			int activity =  select( getSock() + 1 , &rset , &wset , 0 , 0);
			if (activity > 0){
				if(FD_ISSET(getSock(),&wset) ||
					(writeBlockedOnRead && FD_ISSET(getSock(),&rset))){

					writeBlockedOnRead = false;
					int r = SSL_write(_ssl, buf + bpos, length);

					switch(SSL_get_error(_ssl, r)){
					case SSL_ERROR_NONE:
						length -= r;
						bpos += r;
						if( length == 0){
							_busy = false;
							_mutex.unlock();
							return bpos;
						}
						break;
					case SSL_ERROR_WANT_WRITE:
						break;
					case SSL_ERROR_WANT_READ:
						writeBlockedOnRead = true;
						break;
					default:
						ERR_error_string_n(ERR_get_error(), errmsg, sizeof(errmsg));
						LOGWRITE("TLSStack::send() default %s\n",errmsg);
						_busy = false;
						_mutex.unlock();
						return -1;
					}
				}
			}
		}
	}else{
		return TCPStack::send (buf, length);
	}
}


int TLSStack::recv ( uint8_t* buf, uint16_t len ){
	char errmsg[256];
	bool writeBlockedOnRead = false;
	bool readBlockedOnWrite = false;
	bool readBlocked = false;
	int rlen = 0;
	int bpos = 0;
	fd_set rset;
	fd_set wset;


	if(_secureFlg){
		if(_busy){
			return 0;
		}
		_mutex.lock();
		_busy = true;

	loop:
		do{
			readBlockedOnWrite = false;
			readBlocked = false;

			rlen = SSL_read(_ssl, buf + bpos, len - bpos);

			switch (SSL_get_error(_ssl, rlen)){
				case SSL_ERROR_NONE:
					_busy = false;
					_mutex.unlock();
					return rlen + bpos;
					break;
				case SSL_ERROR_ZERO_RETURN:
					SSL_shutdown(_ssl);
					_ssl = 0;
					TCPStack::close();
					_busy = false;
					_mutex.unlock();
					return -1;
					break;
				case SSL_ERROR_WANT_READ:
					readBlocked = true;
					break;
				case SSL_ERROR_WANT_WRITE:
					readBlockedOnWrite = true;
					break;
				default:
					ERR_error_string_n(ERR_get_error(), errmsg, sizeof(errmsg));
					LOGWRITE("TLSStack::recv() default %s\n", errmsg);
					_busy = false;
					_mutex.unlock();
					return -1;
			}
		}while(SSL_pending(_ssl) && !readBlocked);

		bpos += rlen;
		while(true){
			FD_ZERO(&rset);
			FD_ZERO(&wset);
			FD_SET(getSock(), &rset);
			FD_SET(getSock(), &wset);

			int activity =  select( getSock() + 1 , &rset , &wset , 0 , 0);
			if (activity > 0){
				if((FD_ISSET(getSock(),&rset) && !writeBlockedOnRead) ||
					(readBlockedOnWrite && FD_ISSET(getSock(),&wset))){
					goto loop;
				}
			}else{
				ERR_error_string_n(ERR_get_error(), errmsg, sizeof(errmsg));
				LOGWRITE("TLSStack::recv() select %s\n",errmsg);
				_busy = false;
				_mutex.unlock();
				return -1;
			}
		}
	}
	return TCPStack::recv (buf, len);
}


bool TLSStack::isValid(){
	if(!_secureFlg){
		return TCPStack::isValid();
	}
	if(_ssl){
		if(_disconReq){
			SSL_shutdown(_ssl);
			_ssl = 0;
			TCPStack::close();
			_disconReq = false;
		}else{
			return true;
		}
	}
	return false;
}

void TLSStack::disconnect(){
    if (_ssl){
    	_disconReq = true;
    	TCPStack::disconnect();
    }else{
    	TCPStack::disconnect();
    }
}


int TLSStack::getSock(){
	return TCPStack::getSock();
}

SSL* TLSStack::getSSL(){
	if(_secureFlg){
		return _ssl;
	}else{
		return 0;
	}
}

bool TLSStack::isSecure(){
	return _secureFlg;
}
