Async MQTT-SN  over UDP and XBee 
======
*  Async MQTT-SN Gateway running on linux       
*  Async MQTT-SN Client  running on linux and Arduino Uno, Arduino Ethernet.       
*  Design concept of Async client is defferent from Sync one. No more MQTT-SN Message classes.    
*  Program size of Async client for Arduino is 16KB.  previous Sync one is 24KB.    
*  Gateway is changed to support Async PUBACK and SUBACK, returns DISCONNECT.    


Supported functions
-------------------

*  QoS Level 0, 1, 2    
*  SEARCHGW, GWINFO    
*  CONNECT, WILLTOPICREQ, WILLTOPIC, WILLMSGREQ, WILLMSG    
*  PINGREQ, PINGRESP    
*  CONNACK, REGISTER, REGACK, SUBACK, PUBACK, UNSUBACK     
*  SUBSCRIBE, PUBLISH, UNSUBSCRIBE, DISCONNECT    

Implemented control flows:  
   Application program executes PUBLISH() function,   
   Protocol flow as berrow is conducted automaticaly.  


                 Client              Gateway               Broker
      user coding   |                   |                    |      
                    |                   |                    |    
       PUBLISH() -->| --- SERCHGW ----> |                    |  
                    | <-- GWINFO  ----- |                    |  
                    | --- CONNECT ----> |                    |  
                    | <--WILLTOPICREQ-- |                    |  
                    | --- WILLTOPIC --> |                    |  
                    | <-- WILLMSGREQ -- |                    |  
                    | --- WILLMSG ----> | ---- CONNECT ----> |(accepted)     
                    | <-- CONNACK ----- | <--- CONNACK ----- |  
                    | --- SUBSCRIBE --> | ---- SUBSCRIBE --> |     
     [set Callback] | <-- SUBACK ------ | <--- SUBACK ------ |   
                    | --- REGISTER----> |                    |  
                    | <-- REGACK  ----- |                    |  
                    | --- PUBLISH ----> | ---- PUBLISH ----> |      
                    | <-- PUBREC  ----- | <---- PUBREC ----- |    
                    | --- PUBREL  ----> | ----- PUBREL ----> |    
                    | <-- PUBCOMP ----- | <---- PUBCOMP----- |        
                    |                   |                    |        
                    //                  //                   //      
                    | --- PINGREQ ----> | --- PINGREQ ---->  |         
                    | <-- PINGRESP----- | <-- PINGRESP-----  |                    
                    //                  //                   //    
                    |                   |                    |    
                    | <-- REGISTER ---- | <--- PUBLISH ----- |<-- PUBLISH  
                    | --- REGACK  ----> |                    |  
    [exec Callback] | <-- PUBLISH  ---- |                    |  
                    | --- PUBACK   ---> | ---- PUBACK  ----> |--> PUBACK  
                    |                   |                    |  
                    //                  //                   //       
                    |                   |                    |    
    DISCONNECT() -->| ---DISCONNECT---> |                    |  
                    | <--DISCONNECT---- |                    |           
                
License
-------------------
This source code is available under the MIT license 
  

