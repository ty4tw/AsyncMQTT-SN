/*
 * NetworkXBee.cpp
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

#ifdef ARDUINO
    #include <MqttsnClientApp.h>
#else
    #include "MqttsnClientApp.h"
#endif

#ifdef NETWORK_XBEE

#ifdef ARDUINO
  #include <Timer.h>
  #include <NetworkXBee.h>
#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include "NetworkXBee.h"
#endif /* LINUX */

#ifdef LINUX
        #include <stdio.h>
        #include <sys/time.h>
        #include <sys/types.h>
        #include <sys/stat.h>
		#include <sys/ioctl.h>
        #include <unistd.h>
        #include <stdlib.h>
        #include <string.h>
        #include <fcntl.h>
        #include <errno.h>
        #include <termios.h>
#endif /* LINUX */

using namespace std;
using namespace tomyAsyncClient;

extern uint16_t getUint16(const uint8_t* pos);
extern uint32_t getUint32(const uint8_t* pos);
extern void setUint16(uint8_t* pos, uint16_t val);
extern void setUint32(uint8_t* pos, uint32_t val);

/*=========================================
       Class SerialPort
 =========================================*/
#ifdef ARDUINO
/**
 *  For Arduino
 */
SerialPort::SerialPort(){
	_serialDev = 0;
	pinMode(XB_SLEEP_PIN, OUTPUT);
	digitalWrite(XB_SLEEP_PIN, LOW);
}

int SerialPort::open(XBeeConfig config){ //Port num overload.
	if (config.portNo == 0){
	Serial.begin(config.baudrate);
	_serialDev = (Stream*) &Serial;
	}
	#if defined(UBRR1H)
	else if (config.portNo == 1){
	Serial1.begin(config.baudrate);
	_serialDev = (Stream*) &Serial1;
	}
	#endif
	#if defined(UBRR2H)
	else if (config.portNo == 2){
	Serial2.begin(config.baudrate);
	_serialDev = (Stream*) &Serial2;
	}
	#endif
	#if defined(UBRR3H)
	else if (config.portNo == 3){
	Serial3.begin(config.baudrate);
	_serialDev = (Stream*) &Serial3;
	}
	#endif
	return 0;
}

bool SerialPort::checkRecvBuf(){
    return _serialDev->available() > 0;
}

bool SerialPort::send(unsigned char b){
	while(true){
		if(digitalRead(XB_CTS_PIN) == LOW){
			break;
		}
	}
	if(_serialDev->write(b) != 1){
	  return false;
	}else{
	  D_NWSTACK(" ");
	  D_NWSTACK(b,HEX);
	  return true;
	}
}


bool SerialPort::recv(unsigned char* buf){
    if ( _serialDev->available() > 0 ){
        buf[0] = _serialDev->read();

        D_NWSTACK(" ");
        D_NWSTACK(*buf,HEX);
        return true;

    }else{
        return false;
    }
}


void SerialPort::flush(void){
    _serialDev->flush();
}

#endif /* ARDUINO */


#ifdef LINUX

SerialPort::SerialPort(){
    _tio.c_iflag = IGNBRK | IGNPAR;
#ifdef XBEE_FLOWCTRL_CRTSCTS
    _tio.c_cflag = CS8 | CLOCAL | CREAD | CRTSCTS;
#else
    _tio.c_cflag = CS8 | CLOCAL | CREAD;
#endif
    _tio.c_cc[VINTR] = 0;
    _tio.c_cc[VTIME] = 0;
    _tio.c_cc[VMIN] = 0;
    _fd = 0;
}

SerialPort::~SerialPort(){
  if (_fd){
      close(_fd);
  }
}

int SerialPort::open(XBeeConfig config){
  _fd = ::open(config.device, O_RDWR | O_NOCTTY);
  if(_fd < 0){
	  printf("Can't open %s\n", config.device);
      return _fd;
  }
/*
  if (parity){
      _tio.c_cflag = _tio.c_cflag | PARENB;
  }
  if (stopbit == 2){
      _tio.c_cflag = _tio.c_cflag | CSTOPB ;
  }
*/
  	long br;
  	switch (config.baudrate){
  	case 9600:
  		br = B9600;
  		break;
  	case 19200:
  		br = B19200;
  		break;
  	case 38400:
  		br = B38400;
  		break;
  	case 57600:
		br = B57600;
		break;
	case 115200:
		br = B115200;
		break;
	default:
		return -1;
  	}
	if( cfsetspeed(&_tio, br)<0){
		return errno;
	}
    return tcsetattr(_fd, TCSANOW, &_tio);
}

bool SerialPort::checkRecvBuf(){
    return true;
}

bool SerialPort::send(unsigned char b){
  if (write(_fd, &b,1) != 1){
      return false;
  }else{
      D_NWSTACKF( " %x", b);
      return true;
  }
}

bool SerialPort::recv(unsigned char* buf){
  if(read(_fd, buf, 1) == 0){
      return false;
  }else{
	  D_NWSTACKF( " %x",*buf );
      return true;
  }
}

void SerialPort::flush(void){
  tcsetattr(_fd, TCSAFLUSH, &_tio);
}

#endif


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
    resetGwAddress();
}

Network::~Network(){
	delete _serialPort;
}

int Network::initialize(XBeeConfig config){
	return _serialPort->open(config);
}

void Network::setSleep(){
	_sleepflg = true;
}

void Network::setGwAddress(){
    _gwAddrMsb = getUint32(_responseData + 4);
    _gwAddrLsb = getUint32(_responseData + 8);
    _gwAddr16 = getUint16(_responseData + 12);
}

void Network::resetGwAddress(void){
    _gwAddrMsb = 0;
    _gwAddrLsb = 0;
    _gwAddr16 = 0;
}

void Network::setSerialPort(SerialPort *serialPort){
  _serialPort = serialPort;
}

uint8_t* Network::getResponce(int* len){
	memset(_responseData, 0, MQTTSN_MAX_PACKET_SIZE + 1);
	if(_serialPort->checkRecvBuf()){
		if(readApiFrame(PACKET_TIMEOUT_CHECK)){
			if(_responseData[API_ID_POS] == XB_API_RESPONSE){
				*len = _respLen - 15;
				_mqttsnMsg = _responseData + 15;
				return _mqttsnMsg;
			}
		}
	}
	return 0;
}

bool Network::readApiFrame(uint16_t timeoutMillsec){
    _pos = 0;
    _tm.start((uint32_t)timeoutMillsec);

    while (!_tm.isTimeUp()){

        readApiFrame();

        if (_available){
            D_NWSTACKW("<=== CheckSum OK\r\n\n");
            if (_responseData[API_ID_POS] == XB_API_RESPONSE){
				if (_gwAddr16 &&
					(_responseData[14] & 0x02 ) != 0x02 &&
					(_gwAddrMsb != getUint32(_responseData + 4) &&
					(_gwAddrLsb != getUint32(_responseData + 8)))){
					D_NWSTACKW("  Sender is not Gateway!\r\n" );
					return false;
				}
            	return true;
            }else if (_responseData[API_ID_POS] == XB_API_MODEMSTATUS){
            	return true;
            }
        }else if (_errorCode){
            D_NWSTACKW("<=== Packet Error Code = ");
            D_NWSTACKLN(_errorCode, DEC);
            D_NWSTACKF("%d\r\n",_errorCode);
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
            D_NWSTACKW("\r\n===> Recv:    ");
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

        if (_pos >= API_ID_POS){
            _checksumTotal += _byteData;
        }
        if ( _pos > 0){
            if (_pos > MQTTSN_MAX_MSG_LENGTH){
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
	send(payload, (uint8_t) payloadLen, 0);
	return 1;
}

int Network:: unicast(const uint8_t* payload, uint16_t payloadLen){
	send(payload, (uint8_t) payloadLen, 1);
	return 1;
}

void Network::send(const uint8_t* payload, uint8_t pLen, uint8_t unicast){
    D_NWSTACKW("\r\n===> Send:    ");
    uint8_t checksum = 0;
    uint8_t addrBuff[4];

    write(START_BYTE);
    sendByte(0x00);              // Message Length
    sendByte(14 + pLen);         // Message Length

    sendByte(XB_API_REQUEST);    // API
    checksum+= XB_API_REQUEST;

    write(0x00);                 // Frame ID

    if (unicast){
    	setUint32(addrBuff, _gwAddrMsb); // Gateway Address64
    	sendAddr(addrBuff, 4, &checksum);

    	setUint32(addrBuff, _gwAddrLsb);
    	sendAddr(addrBuff, 4, &checksum);

		setUint16(addrBuff, _gwAddr16);
		sendAddr(addrBuff, 2, &checksum);
    }else{
		for (uint8_t i = 0; i < 6; i++){
			write(0x00);   // Broadcast Address64  00 00 00 00 00 00
		}
		for (uint8_t i = 0; i < 3; i++){
			write(0xFF);   // Broadcast Address64  FF FF
			checksum += 0xFF;
		}
		write(0xFE);       // Broadcast Address16  FF FE
		checksum += 0xFE;
    }

    sendByte(0x00);   // Broadcast Radius

    sendByte(0x00);   // Option: Use the extended transmission timeout 0x40
    checksum += 0x00;

    D_NWSTACKW("\r\n     Payload: ");

    for ( int i = 0; i < pLen; i++ ){
        sendByte(payload[i]);     // Payload
        checksum += payload[i];
    }
    checksum = 0xff - checksum;
    sendByte(checksum);
    D_NWSTACKW("\r\n");
    _serialPort->flush();
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

#endif  /* NETWORK_XBEE */
