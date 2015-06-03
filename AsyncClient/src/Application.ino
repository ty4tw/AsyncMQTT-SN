#include <LowPower.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetUdp.h>

#include <Payload.h>
#include <NetworkXBee.h>
#include <MqttsnClient.h>
#include <SubscribeManager.h>
#include <RegisterManager.h>
#include <TopicTable.h>
#include <TaskManager.h>
#include <NetworkUdp.h>
#include <PublishManager.h>
#include <Timer.h>
#include <MqttsnClientApp.h>
#include <GwProxy.h>

#include <SoftwareSerial.h>

#if defined(DEBUG_NW) || defined(DEBUG_MQTTSN) || defined(DEBUG)
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

#ifdef NETWORK_UDP
UDP_APP_CONFIG = {
    {
        "ArduinoEther",
        {225,1,1,1},         // Multicast group IP
        1883,                // Multicast group Port
        {192,168,11,18},     // Local IP     (for Arduino App)
        12001,               // Local PortNo
        {0x90,0xa2,0xda,0x0f,0x53,0xa5}       // MAC address  (for Arduino App)
    },
    {
        300,            //KeepAlive
        true,           //Clean session
        false,           //EndDevice
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
  Payload* pl = new Payload(10);
  onoffFlg = !onoffFlg;
  pl->set_bool(onoffFlg);
  PUBLISH(topic1,pl,2);
}


/*---------------  List of task invoked by Timer ------------*/

TASK_LIST = {//TASK( const char* topic, execution interval in seconds),
             TASK(task1,1),
             END_OF_TASK_LIST
            };

/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

int on_publish(tomyAsyncClient::Payload* payload){
    INDICATOR_ON(payload->get_bool(0));
    return 0;
}

/*------------ Link Callback to Topic -------------*/

SUBSCRIBE_LIST = {//SUB(topic, callback, QoS=0or1),
                  SUB(topic1, on_publish, 1),
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
  #if defined(ARDUINO) && (defined(DEBUG_NW) || defined(DEBUG_MQTTSN) || defined(DEBUG))
	debug.begin(9600);
  #endif

  pinMode(ARDUINO_LED_PIN, OUTPUT);
 }



