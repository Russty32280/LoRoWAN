// ESP32 LORA NCAP
// This code is deisgned to run on an ESP32 attached via Software SPI (VSPI) to an RFM95 LoRa module

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

#include <WiFi.h>
#include <AWS_IOT.h>

#define SERVER_ADDRESS 2 // This is the permenent address of the server within our network. This needs to be changed to reflect our actual choice in address
#define REGISTRATION_ADDRESS 0 // All new Transducer Nodes must start by sending their connection message to Address 0



RH_RF95 driver(16, 17); // Generate a RadioHead Object using pins 16 and 17 for reset and CS

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);


//=============================
// AWS and WiFi Initialization
//=============================

AWS_IOT hornbill;   // AWS_IOT instance


char WIFI_SSID[]="Rowan_IOT";
char WIFI_PASSWORD[]="";
char HOST_ADDRESS[]="a213qaaw8pshxz.iot.us-east-2.amazonaws.com";
char CLIENT_ID[]= "LoRoWANESP32";
char TOPIC_NAME[]= "NetworkNode1/General";

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];


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

}

uint8_t data[] = "9";
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];


void loop() {
  if (manager.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (manager.recvfromAck(buf, &len, &from))
    {
      if (from == REGISTRATION_ADDRESS)
      {
        Serial.println("New device acknowledged. Requesting to change address to 9");
        sprintf(payload, "New Device Acknowledged");
      }
      else if (from == 9)
      {
        Serial.println("Device acknowleged at address 9.");
        sprintf(payload, "Device acknowledged at address 9");
      }
      else
      {
        Serial.print("got request from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(driver.lastRssi(), DEC);
        sprintf(payload, "Got a request from some other device");
      }
      // Send a reply back to the originator client
      
      if (!manager.sendtoWait(data, sizeof(data), from))
      {
        Serial.println("sendtoWait failed");
        sprintf(payload, "sendtoWait failed");
      }

      
     if(hornbill.publish(TOPIC_NAME,payload) == 0)   // Publish the message(Temp and humidity)
        {        
            Serial.print("Publish Message:");   
            Serial.println(payload);
        }
        else
        {
            Serial.println("Publish failed");
        }
    }
  }

}
