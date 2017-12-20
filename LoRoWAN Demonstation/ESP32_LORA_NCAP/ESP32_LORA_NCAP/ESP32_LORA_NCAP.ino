// ESP32 LORA NCAP
// This code is deisgned to run on an ESP32 attached via Software SPI (VSPI) to an RFM95 LoRa module

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#include <WiFi.h>
#include <AWS_IOT.h>

#include <ArduinoJson.h>

#define SERVER_ADDRESS 2 // This is the permenent address of the server within our network. This needs to be changed to reflect our actual choice in address
#define REGISTRATION_ADDRESS 0 // All new Transducer Nodes must start by sending their connection message to Address 0



RH_RF95 driver(16, 17); // Generate a RadioHead Object using pins 16 and 17 for reset and CS
RHReliableDatagram manager(driver, SERVER_ADDRESS);


//=============================
// AWS and WiFi Initialization
//=============================

AWS_IOT hornbill;   // AWS_IOT instance


char WIFI_SSID[]="Rowan_IOT";
char WIFI_PASSWORD[]="";
char HOST_ADDRESS[]="a213qaaw8pshxz.iot.us-east-1.amazonaws.com";
char CLIENT_ID[]= "LoRoWAN2";
char TOPIC_NAME[]= "NetworkNode1/General";

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[251];



//=============================
// Network Initialization
//=============================

//char NextTransducerNodeAddress = 1;   // Keeps track of how many Transducer Nodes are currently on the network


void setup() {
    Serial.begin(115200); // Initialize UART to 115200 Baud
    delay(2000);

    
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        status = WiFi.begin(WIFI_SSID);
        delay(5000);
    }

    Serial.println("Connected to wifi");

    if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0) // Connect to AWS using Host Address and Cliend ID
    {
        Serial.println("Connected to AWS");
        delay(1000);
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }

    delay(2000);
    


    if (!manager.init())
    Serial.println("init failed");

    driver.setTxPower(23, false);
    driver.setModemConfig(RH_RF95::ModemConfigChoice::Bw500Cr45Sf128);
    driver.setFrequency(915.0);
    manager.setTimeout(1000);
    Serial.println("Passed Init");
    
    /*************************
    * Example JSON to AWS
    **************************/

    
    /*
    JsonObject& root = jsonBuffer.createObject();
    root["FNID"] = "7211";
    root["NNID"] = "1";
    root["TNID"] = "1";
    root["ChanID"] = "1";
    root["Data"] = "1";
    root["temp"] = "85";
  
    root.printTo(payload, sizeof(payload));

     if(hornbill.publish(TOPIC_NAME,payload) == 0)   // Publish the message(Temp and humidity)
        {        
            Serial.print("Publish Message:");   
            Serial.println(payload);
        }
        else
        {
            Serial.println("Publish failed");
        }
           

    */

}

char data[512];
uint8_t buf[100];

bool IsSending = true;

uint16_t FNID;
uint16_t NNID;
uint16_t TNID;
uint16_t ChanID;


uint8_t len = sizeof(buf);
uint8_t from;

//JsonObject& root = jsonBuffer.createObject();

void loop() {
    Serial.println("Starting Loop");

    StaticJsonBuffer<200> jsonBuffer;
    StaticJsonBuffer<200> jsonBuffer2;
    StaticJsonBuffer<200> jsonBuffer3;


    if (IsSending)
    {
      
      Serial.println("Json Object");
      JsonObject& root = jsonBuffer.createObject();
      Serial.println("Created JSOn Object for Root1");
      root["FNID"] = 7211;
      root["NNID"] = 1;
      root["TNID"] = 1;
      root["ChanID"] = 1;
      root["Data"] = 1;
      root["temp"] = 85; 

      Serial.println("Before PrintTo Root1");
      root.printTo(data, sizeof(data));
      Serial.println("After PrintTo Root1");
      /*
      FNID = 7211;
      TNID = 1;
      NNID = 1;
      ChanID = 1;

      data[0] = (uint8_t)(FNID&0x00FF);   // Set first byte of data to be sent as the FuncID High Byte  
      data[1] = (uint8_t)((FNID&0xFF00)>>8);   // Set Second byte to FuncID Low Byte
      data[2] = (uint8_t)(NNID&0x00FF);;   // Next, the Network Node ID which requested the data
      data[3] = (uint8_t)((NNID&0xFF00)>>8);
      data[4] = (uint8_t)(TNID&0x00FF);  // Transducer Node High Byte
      data[5] = (uint8_t)((TNID&0xFF00)>>8);  // Transducer Node Low Byte
      data[6] = (uint8_t)(ChanID&0x00FF);  // Channel ID High
      data[7] = (uint8_t)((ChanID&0xFF00)>>8);   //  Channel ID Low
      */

      
      Serial.println(data);
      Serial.println("Sending Data");
      //uint8_t intData = (uint8_t)data;
      if (!manager.sendtoWait((uint8_t*)data, sizeof(data), 1))
      {
        Serial.println("sendtoWait failed");
        sprintf(payload, "sendtoWait failed");
      }
      else
        Serial.println("Else, change to recieving");
        IsSending = false;
    }
  else
  {
      Serial.println("Receving Mode");

      while (!(manager.recvfromAck(buf, &len, &from)))
      {
        //Serial.println("Waiting");
      }
      Serial.println("Got Something");
      IsSending = true;
      Serial.println("Changed IsSending");

     //buf is huge!!
     //This reads important stuff from buff
     //And repackages into smaller JSON
    
    JsonObject& root2 = jsonBuffer2.parseObject((char*)buf);
    Serial.println("Generated Root2");
    uint16_t FunctionID = root2["FNID"];
    uint16_t NNID = root2["NNID"];
    uint16_t TNID = root2["TNID"];
    uint8_t ChanID = root2["ChanID"];
    uint32_t Data = root2["Data"];

    /*
    JsonObject& root3 = jsonBuffer.createObject();
    Serial.println("Created Root3")
    root3["FNID"] = buf[0]<<8 + buf[1];
    root3["NNID"] = buf[2]<<8 + buf[3];
    root3["TNID"] = buf[4]<<8 + buf[5];
    root3["ChanID"] = buf[6]<<8 + buf[7];
    root3["Data"] = buf[8]<<8 + buf[9];
    root3["temp"] = "85";

    root3.printTo(payload, sizeof(payload));
    Serial.println(payload);
    */
    
    JsonObject& root3 = jsonBuffer3.createObject();
    Serial.println("Generated Root3");
    root3["FNID"] = FunctionID;
    root3["NNID"] = NNID;
    root3["TNID"] = TNID;
    root3["ChanID"] = ChanID;
    root3["Data"] = Data;
    root3["temp"] = "85";
  
    root3.printTo(payload, sizeof(payload));
       
     if(hornbill.publish(TOPIC_NAME,payload) == 0)   // Publish the message(Temp and humidity)
        {        
            Serial.print("Publish Message:");   
            //Serial.println(buf);
        }
        else
        {
            Serial.println("Publish failed");
        }
  }
    delay(1000);
}
