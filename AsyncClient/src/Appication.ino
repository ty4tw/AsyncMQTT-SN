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
 */
#include <MqttsnClientApp.h>
#include <MqttsnClient.h>
using namespace std;
using namespace tomyAsyncClient;
extern MqttsnClient* theClient;

/*============================================
 *
 *      MQTT-SN Client Application
 *
 *===========================================*/
 XBEE_APP_CONFIG = {
    {
        "clientFio01",     //ClientId
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
  tomyAsyncClient::Payload* pl = new Payload(10);
  onoffFlg = !onoffFlg;
  pl->set_bool(onoffFlg);
  PUBLISH(topic2,pl,1);
}

void task2(void){
  
}

/*---------------  List of task invoked by Timer ------------*/

TASK_LIST = {  //TASK( const char* topic, executing duration in second),
             TASK(task1,3),
             TASK(task2,20),
             END_OF_TASK_LIST
            };

/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

int on_publish(tomyAsyncClient::Payload* pload){
    //printf("ON_PUBLISH invoked.  ");
    INDICATOR_ON(pload->get_bool(0));
    return 0;
}

/*------------ Link Callback to Topic -------------*/

SUBSCRIBE_LIST = {//SUB(topic, callback, QoS=0or1),
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



