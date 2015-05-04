/*
 * ZBeeStack.cpp
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
#ifdef  NETWORK_XBEE

#include "Messages.h"
#include "ZBStack.h"
#include "ProcessFramework.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

using namespace std;
using namespace tomyAsyncGateway;

extern uint8_t* mqcalloc(uint8_t length);
extern uint16_t getUint16(uint8_t* pos);
extern uint32_t getUint32(uint8_t* pos);
extern void setUint16(uint8_t* pos, uint16_t val);
extern void setUint32(uint8_t* pos, uint32_t val);

/*===========================================
              Class  Network
 ============================================*/
Network::Network(){
    _serialPort = new SerialPort();
    _returnCode = 0;
    _tm.stop();
    _pos = 0;
    _esc = false;
    _checksumTotal = 0;
    _sleepflg = false;
}

Network::~Network(){
	delete _serialPort;
}

int Network::initialize(XBeeConfig config){
	return _serialPort->open(config);
}

void Network::setSerialPort(SerialPort *serialPort){
  _serialPort = serialPort;
}

uint8_t* Network::getResponce(int* len){
	if(readApiFrame(PACKET_TIMEOUT_CHECK)){
		if(_responseData[ZB_API_ID_POS] == XB_API_RESPONSE){
			*len = _respLen - ZB_RX_PAYLOAD_OFFSET;
			_mqttsnMsg = _responseData + ZB_RX_PAYLOAD_OFFSET;
			return _mqttsnMsg;
		}
	}
	return 0;
}

uint32_t Network::getAddrMsb(void){
	return getUint32(_responseData + 4);
}

uint32_t Network::getAddrLsb(void){
	return getUint32(_responseData + 8);
}

uint16_t Network::getAddr16(void){
	return getUint16(_responseData + 12);
}

bool Network::readApiFrame(uint16_t timeoutMillsec){
    _pos = 0;
    _tm.start((uint32_t)timeoutMillsec);

    while (!_tm.isTimeup()){

        readApiFrame();

        if (_available){
        	D_NWSTACK("<=== CheckSum OK\r\n\n");
            if (_responseData[ZB_API_ID_POS] == XB_API_RESPONSE){
				return true;
            }else if (_responseData[ZB_API_ID_POS] == XB_API_MODEMSTATUS){
            	return true;
            }
        }else if (_errorCode){
        	D_NWSTACK("\r\n<=== Packet Error Code = %d\r\n",_errorCode);
            return false;
        }
    }
    return false;   //Timeout
}

void Network::readApiFrame(){

    if (_available || _errorCode){
      _available = _errorCode = _pos = 0;
    }

    while (read(&_byteData )){

        if ( _byteData == START_BYTE){
            _pos = 1;
            D_NWSTACK("\r\n===> Recv:    ");
            continue;
        }

        if (_pos > 0 && _byteData == ESCAPE){
          if (read(&_byteData )){
              _byteData = 0x20 ^ _byteData;  // decode
          }else{
              _esc = true;
              continue;
          }
        }

        if (_esc){
            _byteData = 0x20 ^ _byteData;
            _esc = false;
        }

        if (_pos >= ZB_API_ID_POS){
            _checksumTotal += _byteData;
        }
        if ( _pos > 0){
            if (_pos > MAX_FRAME_DATA_SIZE){
            	_errorCode = PACKET_OVERFLOW;
              _pos = 0;
              return;
            }else if (_pos > 2 && _pos == _responseData[2] + 3){
              if ((_checksumTotal & 0xff) == 0xff){
                  _available = 1;
                  _errorCode = NO_ERROR;
              }else{
            	  _errorCode = CHECKSUM_ERROR;
              }
              _respLen = _pos;
              _pos = 0;
              _checksumTotal = 0;
              return;
            }else{
            	_responseData[_pos] = _byteData;
            }
        }
        _pos++;


    }
}

int Network::broadcast(const uint8_t* payload, uint16_t payloadLen){
	send(payload, (uint8_t) payloadLen);
	return 1;
}

int Network:: unicast(const uint8_t* payload, uint16_t payloadLen, uint32_t msb, uint32_t lsb, uint16_t addr16){
	send(payload, (uint8_t) payloadLen, msb, lsb, addr16);
	return 1;
}

void Network::send(const uint8_t* payload, uint8_t pLen, uint32_t msb, uint32_t lsb, uint16_t addr16){
	D_NWSTACK("\r\n===> Send:    ");
    uint8_t checksum = 0;
    uint8_t addrBuff[4];

    write(START_BYTE);
    sendByte(0x00);              // Message Length
    sendByte(14 + pLen);         // Message Length

    sendByte(XB_API_REQUEST);    // API
    checksum+= XB_API_REQUEST;

    write(0x00);                 // Frame ID

	setUint32(addrBuff, msb); // Gateway Address64
	sendAddr(addrBuff, 4, &checksum);

	setUint32(addrBuff, lsb);
	sendAddr(addrBuff, 4, &checksum);

	setUint16(addrBuff, addr16);
	sendAddr(addrBuff, 2, &checksum);

    sendByte(0x00);   // Broadcast Radius

    sendByte(0x00);   // Option: Use the extended transmission timeout 0x40
    checksum += 0x00;

    D_NWSTACK("\r\n     Payload: ");

    for ( int i = 0; i < pLen; i++ ){
        sendByte(payload[i]);     // Payload
        checksum += payload[i];
    }
    checksum = 0xff - checksum;
    sendByte(checksum);
    D_NWSTACK("\r\n");
}

void Network::sendAddr(uint8_t* addr, uint8_t len, uint8_t* checksum){
	for (int i = 0; i < len; i++){
		sendByte(addr[i]);   // Gateway Address 64 & 16
		*checksum += addr[i];
	}
}

void Network::sendByte(uint8_t b){
  if(b == START_BYTE || b == ESCAPE || b == XON || b == XOFF){
      write(ESCAPE);
      write(b ^ 0x20);
  }else{
      write(b);
  }
}

void Network::write(uint8_t val){
	_serialPort->send(val);
}

bool Network::read(uint8_t *buff){
	return  _serialPort->recv(buff);
}

/*=========================================
       Class SerialPort
 =========================================*/
SerialPort::SerialPort(){
    _tio.c_iflag = IGNBRK | IGNPAR;
    _tio.c_cflag = CS8 | CLOCAL | CREAD | CRTSCTS;
    _tio.c_cc[VINTR] = 0;
    _tio.c_cc[VTIME] = 0;
    _tio.c_cc[VMIN] = 1;
    _fd = 0;
}

SerialPort::~SerialPort(){
	  if (_fd){
		  close(_fd);
	  }
}

int SerialPort::open(XBeeConfig config){
  return open(config.device, config.baudrate, false, 1, config.flag);
}

int SerialPort::open(const char* devName, unsigned int baudrate,  bool parity, unsigned int stopbit, unsigned int flg){
	_fd = ::open(devName, flg | O_NOCTTY);
	if(_fd < 0){
	  return _fd;
	}

	if (parity){
	  _tio.c_cflag = _tio.c_cflag | PARENB;
	}
	if (stopbit == 2){
	  _tio.c_cflag = _tio.c_cflag | CSTOPB ;
	}
	switch(baudrate){
	case B9600:
	case B19200:
	case B38400:
	case B57600:
	case B115200:
	  if( cfsetspeed(&_tio, baudrate)<0){
		return errno;
	  }
	  break;
	default:
	  return -1;
	}
	return tcsetattr(_fd, TCSANOW, &_tio);
}

bool SerialPort::send(unsigned char b){
	if (write(_fd, &b,1) != 1){
	    return false;
	}else{
		D_NWSTACK( " %02x", b);
	    return true;
	}
}

bool SerialPort::recv(unsigned char* buf){
	if(read(_fd, buf, 1) == 0){
	    return false;
	}else{
		D_NWSTACK( " %02x",buf[0] );
	    return true;
	}
}

void SerialPort::flush(void){
	tcsetattr(_fd, TCSAFLUSH, &_tio);
}

#endif /* NETWORK_XBEE */
