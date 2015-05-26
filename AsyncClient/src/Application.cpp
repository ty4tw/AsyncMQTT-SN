/*
 * Application.cpp
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
 *  Created on: 2015/04/19
 *    Modified: 2015/05/16
 *      Author: tomoaki
 *     Version: 0.1.0
 */
#ifdef ARDUINO
#include <lib/MqttsnClientApp.h>
#include <lib/MqttsnClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#else
#include "lib/MqttsnClientApp.h"
#include "lib/MqttsnClient.h"
#endif

#if defined(ARDUINO) && (defined(DEBUG_NW) || defined(DEBUG_MQTTSN) || defined(DEBUG))
#include <SoftwareSerial.h>
SoftwareSerial debug(8, 9);
#endif

using namespace std;
using namespace tomyAsyncClient;
extern MqttsnClient* theClient;

/*============================================
 *
 *      MQTT-SN Client Application
 *
 *===========================================*/
 #ifdef NETWORK_XBEE
 XBEE_APP_CONFIG = {
    {
    	"client01",     //ClientId
        57600,          //Baudrate
        0,              //Serial PortNo (for Arduino App)
        "/dev/ttyUSB0"               //Device (for linux App)
    },
    {
        300,            //KeepAlive
        true,           //Clean session
        false,          //EndDevice
        "ty4tw@github/willTopic",    //WillTopic   or 0   DO NOT USE NULL STRING "" !
        "willMessage"   //WillMessage or 0   DO NOT USE NULL STRING "" !
    }
 };
#endif

#ifdef NETWORK_UDP
UDP_APP_CONFIG = {
    {
      	"LinuxClient",      //ClientId
        {225,1,1,1},        // Multicast group IP
        1883,               // Multicast group Port
        {0,0,0,0},          // Local IP     (for Arduino App)
        12001,              // Local PortNo
        {0,0,0,0,0,0}       // MAC address  (for Arduino App)
    },
    {
        300,            //KeepAlive
        true,           //Clean session
        false,          //EndDevice
        "willTopic",    //WillTopic   or 0   DO NOT USE NULL STRING "" !
        "willMessage"   //WillMessage or 0   DO NOT USE NULL STRING "" !
    }
};
#endif

/*------------------------------------------------------
 *             Create Topic
 *------------------------------------------------------*/
const char* topic1 = "ty4tw@github/onoff/arduino";
const char* topic2 = "ty4tw@github/onoff/linux";
/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/

static bool onoffFlg = true;

void task1(void){
  printf("TASK1 invoked\n");
  Payload* pl = new Payload(10);
  onoffFlg = !onoffFlg;
  pl->set_bool(onoffFlg);
  PUBLISH(topic1,pl,1);
}

void task2(void){
  printf("TASK2 invoked\n");
}

/*---------------  List of task invoked by Timer ------------*/

TASK_LIST = {  //TASK( const char* topic, executing duration in second),
             TASK(task1,10),
             TASK(task2,20),
             END_OF_TASK_LIST
            };

/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

int on_publish(Payload* payload){
    printf("ON_PUBLISH invoked.  ");
    INDICATOR_ON(payload->get_bool(0));
    return 0;
}

/*------------ Link Callback to Topic -------------*/

SUBSCRIBE_LIST = {  //SUB(topic, on_publish, QoS),
                  SUB(topic2, on_publish, 1),
                  END_OF_SUBSCRIBE_LIST
                 };

/*------------------------------------------------------
 *            Tasks invoked by INT0 interruption
 *------------------------------------------------------*/
void interruptCallback(void){

}

/*------------------------------------------------------
 *            setup() function
 *------------------------------------------------------*/
 void setup(void){

 }



