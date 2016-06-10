/*
 * Util.cpp
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

#ifndef ARDUINO
        #include "MqttsnClientApp.h"
#else
        #include <MqttsnClientApp.h>
#endif

using namespace std;
/*=====================================
        Global functions
 ======================================*/
#ifdef ARDUINO
int getFreeMemory()
{
extern char __bss_end;
extern char *__brkval;

  int freeMemory;

  if((int)__brkval == 0)
    freeMemory = ((int)&freeMemory) - ((int)&__bss_end);
  else
    freeMemory = ((int)&freeMemory) - ((int)__brkval);

  return freeMemory;
}
#endif

#ifndef CPU_BIGENDIANN

/*--- For Little endianness ---*/

uint16_t getUint16(const uint8_t* pos){
	uint16_t val = ((uint16_t)*pos++ << 8);
	return val += *pos;
}

void setUint16(uint8_t* pos, uint16_t val){
    *pos++ = (val >> 8) & 0xff;
	*pos   = val & 0xff;
}

uint32_t getUint32(const uint8_t* pos){
    uint32_t val = uint32_t(*pos++) <<  24;
	val += uint32_t(*pos++) << 16;
	val += uint32_t(*pos++) <<  8;
	return val += *pos++;
}

void setUint32(uint8_t* pos, uint32_t val){
	*pos++ = (val >> 24) & 0xff;
	*pos++ = (val >> 16) & 0xff;
	*pos++ = (val >>  8) & 0xff;
	*pos   =  val & 0xff;
}

float getFloat32(const uint8_t* pos){
	union{
		float flt;
		uint8_t d[4];
	}val;
    val.d[3] = *pos++;
	val.d[2] = *pos++;
	val.d[1] = *pos++;
	val.d[0] = *pos;
	return val.flt;
}

void setFloat32(uint8_t* pos, float flt){
	union{
		float flt;
		uint8_t d[4];
	}val;
	val.flt = flt;
    *pos++ = val.d[3];
    *pos++ = val.d[2];
    *pos++ = val.d[1];
    *pos   = val.d[0];
}

#else

/*--- For Big endianness ---*/

uint16_t getUint16(const uint8_t* pos){
  uint16_t val = *pos++;
  return val += ((uint16_t)*pos++ << 8);
}

void setUint16(uint8_t* pos, uint16_t val){
	*pos++ =  val & 0xff;
	*pos   = (val >>  8) & 0xff;
}

uint32_t getUint32(const uint8_t* pos){
    long val = uint32_t(*(pos + 3)) << 24;
    val += uint32_t(*(pos + 2)) << 16;
	val += uint32_t(*(pos + 1)) <<  8;
	return val += *pos;
}

void setUint32(uint8_t* pos, uint32_t val){
    *pos++ =  val & 0xff;
    *pos++ = (val >>  8) & 0xff;
    *pos++ = (val >> 16) & 0xff;
    *pos   = (val >> 24) & 0xff;
}

float getFloat32(const uint8_t* pos){
	union{
		float flt;
		uint8_t d[4];
	}val;

    val.d[0] = *pos++;
	val.d[1] = *pos++;
	val.d[2] = *pos++;
	val.d[3] = *pos;
	return val.flt;
}

void setFloat32(uint8_t* pos, float flt){
	union{
		float flt;
		uint8_t d[4];
	}val;
	val.flt = flt;
    *pos++ = val.d[0];
    *pos++ = val.d[1];
    *pos++ = val.d[2];
    *pos   = val.d[3];
}

#endif  // CPU_LITTLEENDIANN




