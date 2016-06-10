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
  #if defined(DEBUG_NW) || defined(DEBUG_MQTTSN) || defined(DEBUG)
        #include <SoftwareSerial.h>
        extern SoftwareSerial debug;
  #endif
#endif  /* ARDUINO */


#ifdef LINUX
  #include "Timer.h"
  #include "NetworkXBee.h"
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
	  D_NWA(b, HEX);
	  D_NWL(" %x",b);
	  return true;
	}
}


bool SerialPort::recv(unsigned char* buf){
    if ( _serialDev->available() > 0 ){
        buf[0] = _serialDev->read();
        D_NWA(*buf, HEX);
        D_NWL(" %x",*buf);
        return true;

    }else{
        return false;
    }
}


void SerialPort::flush(void){
    _serialDev->flush();
}

void SerialPort::rtsOff(void){
    pinMode(XB_RTS_PIN, INPUT);
}

void SerialPort::rtsOn(void){
    pinMode(XB_RTS_PIN, OUTPUT);
    digitalWrite(XB_RTS_PIN, LOW);
    delay(20);
}


#endif /* ARDUINO */


#ifdef LINUX

SerialPort::SerialPort(){
	memset(&_tio, 0, sizeof(termios));
    _tio.c_iflag = IGNBRK | IGNPAR;
#ifdef XBEE_FLOWCTL_CRTSCTS
    _tio.c_cflag = CS8 | CLOCAL | CRTSCTS;
#else
    _tio.c_cflag = CS8 | CLOCAL;
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
	int rc;
    ioctl(_fd, FIONREAD, &rc);
    return rc > 0 ;
}

bool SerialPort::send(unsigned char b){
  if (write(_fd, &b,1) != 1){
      return false;
  }else{
      D_NWL( " %x", b);
      return true;
  }
}

bool SerialPort::recv(unsigned char* buf){
  if(read(_fd, buf, 1) == 0){
      return false;
  }else{
	  D_NWL( " %x",*buf );
      return true;
  }
}

void SerialPort::flush(void){
  tcsetattr(_fd, TCSAFLUSH, &_tio);
}

void SerialPort::rtsOn(void){

}

void SerialPort::rtsOff(void){

}

#endif


/*===========================================
              Class  Network
 ============================================*/

Network::Network(){
    _serialPort = new SerialPort();
    _returnCode = 0;
    _tm.stop();


    _sleepflg = false;
    resetGwAddress();
    _frameId = 0;
}

Network::~Network(){
	delete _serialPort;
}

int Network::initialize(XBeeConfig config){
	_serialPort->rtsOff();
	return _serialPort->open(config);
}

void Network::setSleep(){
	_sleepflg = true;
}

void Network::setGwAddress(){
    memcpy(_gwAddr64, _responseData + 1, 8);
    memcpy(_gwAddr16, _responseData + 9, 2);

}

void Network::resetGwAddress(void){
	memset(_gwAddr64, 0, 8);
	memset(_gwAddr16, 0, 2);
}

void Network::setSerialPort(SerialPort *serialPort){
  _serialPort = serialPort;
}

uint8_t* Network::getMessage(int* len){
	memset(_responseData, 0, MQTTSN_MAX_PACKET_SIZE + 1);
	_serialPort->rtsOn();

	if(_serialPort->checkRecvBuf()){
		if( (readApiFrame(PACKET_TIMEOUT_CHECK)) ){
			if(_responseData[0] == XB_API_RESPONSE){
				*len = _packetLen - 12;
				return _responseData + 12;
			}
		}
	}
	_serialPort->rtsOff();
	return 0;
}

uint8_t Network::readApiFrame(uint16_t timeoutMillsec)
{
	uint8_t buff;
	uint8_t flg = 0;
    _tm.start((uint32_t)timeoutMillsec);

    while (!_tm.isTimeUp())
    {
    	_serialPort->recv(&buff);
    	if ( buff == START_BYTE)
		{
			_checksum = 0;
			flg = 1;
			D_NWA(F(" ===> Recv:    "));
			D_NWL(" ===> Recv:    ");
			break;
		}
    }

    if ( flg != 1 )
    {
    	return 0;
    }

    /* get packet */
    if ( readApiFrame() > 0 )
    {
    	if (_responseData[0] == XB_API_RESPONSE)
    	{
    		if ( memcmp(_gwAddr64, _responseData + 1, 8) != 0 &&
				(_responseData[12] & 0x02)  != 0x02 )
    		{
				D_NWL("  Sender is not Gateway!\r\n" );
				return 0;
			}
    		else
    		{
    			return 1;  // recieve Response
    		}
    	}
		else if (_responseData[0] == XB_API_MODEMSTATUS)
		{
			return 0;
		}
		else if (_responseData[0] == XB_API_XMITSTATUS)
		{
			return 2;  // recieve Ack
		}
    }
	return 0;
}

uint8_t Network::readApiFrame(){
	uint8_t buf;
	uint8_t pos = 0;
	uint8_t checksum = 0;
	uint8_t len = 0;

   	recvByte(&buf);  // MSB length
    recvByte(&buf);  // LSB length

    len = buf;

    while ( len-- )
    {
    	recvByte(&buf);
    	_responseData[pos++] = buf;
    	checksum += buf;
    }
    _packetLen = 0;

	recvByte(&buf);   // checksum

    if ( (0xff - checksum ) == buf ){
    	D_NWA(F("    checksum ok\r\n"));
    	D_NWL("    checksum ok\r\n");
    	_packetLen = pos;
    	return 1;
    }
    else
    {
    	D_NWA(F("    checksum error\r\n"));
    	D_NWL("    checksum error\r\n");
    	goto errexit;
    }
errexit:
	_serialPort->flush();
	return 0;
}

int Network::broadcast(const uint8_t* payload, uint16_t payloadLen){
	return send(payload, (uint8_t) payloadLen, 0);
}

int Network:: unicast(const uint8_t* payload, uint16_t payloadLen){
	return send(payload, (uint8_t) payloadLen, 1);
}

uint8_t Network::send(const uint8_t* payload, uint8_t pLen, uint8_t unicast){
	D_NWA(F("\r\n===> Send:    "));
	D_NWL("\r\n===> Send:    ");
    uint8_t checksum = 0;

    _serialPort->send(START_BYTE);
    sendByte(0x00);              // Message Length
    sendByte(14 + pLen);         // Message Length

    sendByte(XB_API_REQUEST);    // API
    checksum+= XB_API_REQUEST;

    if ( _frameId++ == 0 )
    {
    	_frameId = 1;
    }
    _serialPort->send(_frameId);                 // Frame ID
    checksum += _frameId;

    if (unicast){
    	for ( uint8_t i = 0; i < 8; i++)
    	{
    		_serialPort->send(_gwAddr64[i]);
    		checksum += _gwAddr64[i];
    	}
    	for ( uint8_t i = 0; i < 2; i++)
		{
			_serialPort->send(_gwAddr16[i]);
			checksum += _gwAddr16[i];
		}
    }else{
		for (uint8_t i = 0; i < 6; i++){
			_serialPort->send(0x00);   // Broadcast Address64  00 00 00 00 00 00
		}
		for (uint8_t i = 0; i < 3; i++){
			_serialPort->send(0xFF);   // Broadcast Address64  FF FF
			checksum += 0xFF;
		}
		_serialPort->send(0xFE);       // Broadcast Address16  FF FE
		checksum += 0xFE;
    }

    sendByte(0x00);   // Broadcast Radius

    sendByte(0x00);   // Option: Use the extended transmission timeout 0x40
    checksum += 0x00;

    D_NWA(F("\r\n     Payload: "));
    D_NWL("\r\n     Payload: ");

    for ( int i = 0; i < pLen; i++ ){
        sendByte(payload[i]);     // Payload
        checksum += payload[i];
    }
    checksum = 0xff - checksum;
    sendByte(checksum);
    D_NWALN();
    D_NWL("\r\n");

    if (unicast){
		if ( readApiFrame(4000) == 2 )  // expect Xmit status message while 4 secs
		{
			return 1;
		}
		else
		{
			D_NWA(F("XmitStatus Timeout\r\n"));
			D_NWL("XmitStatus Timeout\r\n");
			return 0;
		}
    }
}

void Network::sendByte(uint8_t b){
  if(b == START_BYTE || b == ESCAPE || b == XON || b == XOFF){
	  _serialPort->send(ESCAPE);
	  _serialPort->send(b ^ 0x20);
  }else{
	  _serialPort->send(b);
  }
}

int Network::recvByte(uint8_t* buf)
{
	bool flg;
	_tm.start(1000);
	do
	{
		flg = _serialPort->recv(buf);
	}
	while ( !flg && !_tm.isTimeUp() );

	if ( flg )
	{
		if ( *buf == ESCAPE)
		{
			_tm.start(1000);
			do
			{
				flg = _serialPort->recv(buf);
			}
			while ( !flg && !_tm.isTimeUp() );
			if ( !flg )
			{
				return -1;
			}
			*buf = 0x20 ^ *buf;
		}
		return 0;
	}
	return -1;
}


#endif  /* NETWORK_XBEE */
