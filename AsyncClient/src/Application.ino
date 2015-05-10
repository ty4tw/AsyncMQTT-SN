
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
 #ifdef NETWORK_XBEE
 XBEE_APP_CONFIG = {
    {
        "ArduinoNano01", 
        9600,           //Baudrate
        0,              //Serial PortNo (for Arduino App)
        0               //Device (for linux App)
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

#ifdef NETWORK_UDP
UDP_APP_CONFIG = {
    {
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
const char* topic1 = "ty4tw/tp1";
const char* topic2 = "ty4tw/tp2";
const char* tpMeasure = "ty4tw/soilReg";
/*------------------------------------------------------
 *             Tasks invoked by Timer
 *------------------------------------------------------*/
#define PIN5  5    // 3.3V supply port
#define MCOUNT 10  // measurement count
#define PIN0  0    // measurement port
#define RP   20    // resistance [K ohom]

void measure(){
  int val = 0;
  pinMode(PIN5,OUTPUT);
  digitalWrite(PIN5,1);

  for(uint8_t cnt = 0; cnt < MCOUNT; cnt++){
    delay(50);
    val += analogRead(PIN0);
  }
  digitalWrite(PIN5,0);
  int soilR = 1023 * RP / (val /MCOUNT) - RP;
  if(soilR < 0){
    soilR = 9999;
  }

  Payload* pl = new Payload(30);
  pl->set_array(3);
  pl->set_uint32(GETUTC());
  pl->set_int32(100);
  pl->set_str("Kohom");
  PUBLISH(tpMeasure,pl,1);
}


void task1(){
  Payload* pl = new Payload(36);
  pl->set_array(9);
  pl->set_int32(30);
  pl->set_int32(255);
  pl->set_int32(70000);
  pl->set_str("abcdef");
  pl->set_int32(-16);
  pl->set_int32(-60);
  pl->set_int32(-300);
  pl->set_int32(-70000);
  pl->set_float(1000.01);
  PUBLISH(topic1,pl,1);
}

/*---------------  List of task invoked by Timer ------------*/

TASK_LIST = {  //TASK( const char* topic, executing duration in second),
           TASK(measure, 40),
           TASK(task1,6),
END_OF_TASK_LIST};


/*------------------------------------------------------
 *       Tasks invoked by PUBLISH command Packet
 *------------------------------------------------------*/

int on_publish2(tomyAsyncClient::Payload* pl){
    //theApplication->indicatorOff();
    digitalWrite(133,1);
    delay(500);
    digitalWrite(13,0);
    return 0;
}

/*------------ Link Callback to Topic -------------*/

SUBSCRIBE_LIST = {
                    //SUB(topic1, on_publish1, QOS1),
                    SUB(topic2, on_publish2, 1),
END_OF_SUBSCRIBE_LIST};

/*------------------------------------------------------
 *            Tasks invoked by INT0 interuption
 *------------------------------------------------------*/
void interruptCallback(){
  //NOP
}

/*------------------------------------------------------
 *            setup() function
 *------------------------------------------------------*/
 void setup(){

 }


