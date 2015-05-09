Async MQTT-SN  over UDP and XBee 
======
*  Async MQTT-SN Gateway running on linux       
*  Async MQTT-SN Client  running on linux and Arduino Uno, Arduino Ehernet       
*  Program size of Async client for Arduino is 15KB.  previous Sync one is 24KB.
*  Design concept of Async client is defferent from Sync one. No more MQTT-SN Message classes.
  

Supported functions
-------------------

*  QOS Level 0, 1, 2    
*  SEARCHGW, GWINFO    
*  CONNECT, WILLTOPICREQ, WILLTOPIC, WILLMSGREQ, WILLMSG    
*  PINGREQ, PINGRESP    
*  CONNACK, REGISTER, REGACK, SUBACK, PUBACK, UNSUBACK     
*  SUBSCRIBE, PUBLISH, UNSUBSCRIBE, DISCONNECT    

Implemented control flows:  
   Application program executes PUBLISH() function,   
   Protocol flow as berrow is conducted automaticaly.  


                 Client              Gateway               Broker
                    |                   |                    |      
       PUBLISH() -->| --- SERCHGW ----> |                    |  
                    | <-- GWINFO  ----- |                    |  
                    | --- CONNECT ----> |                    |  
                    | <--WILLTOPICREQ-- |                    |  
                    | --- WILLTOPIC --> |                    |  
                    | <-- WILLMSGREQ -- |                    |  
                    | --- WILLMSG ----> | ---- CONNECT ----> |(accepted)     
                    | <-- CONNACK ----- | <--- CONNACK ----- |   
                    | --- PUBLISH ----> |                    |  
                    | <-- PUBACK  ----- | (invalid TopicId)  |  
                    | --- REGISTER ---> |                    |  
                    | <-- REGACK  ----- |                    |  
                    | --- PUBLISH ----> | ---- PUBLISH ----> |(accepted)  
                    | <-- PUBACK  ----- | <---- PUBACK ----- |    
                    |                   |                    |    
                    //                  //                   //      
                    |                   |                    |          
     SUBSCRIBE() -->| --- SUBSCRIBE --> | ---- SUBSCRIBE --> |     
     [set Callback] | <-- SUBACK ------ | <--- SUBACK ------ |    
                    |                   |                    |    
                    //                  //                   //    
                    |                   |                    |    
                    | <-- REGISTER ---- | <--- PUBLISH ----- |<-- PUBLISH  
    [exec Callback] | <-- PUBLISH  ---- |                    |  
                    | --- PUBACK   ---> | ---- PUBACK  ----> |--> PUBACK  
                    |                   |                    |  
                
License
-------------------
This source code is available under the MIT license 
  

