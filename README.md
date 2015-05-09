Async MQTT-SN  over UDP and XBee
======
*  Async MQTT-SN Gateway over XBee or UDP (running on linux)     
*  Async MQTT-SN Client over UDP  (running on linux and Arduino)    
 
  

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
  

