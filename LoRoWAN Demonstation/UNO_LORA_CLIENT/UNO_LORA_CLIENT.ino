#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <ArduinoJson.h>

//#define CLIENT_ADDRESS 1
uint8_t CLIENT_ADDRESS = 1;

#define SERVER_ADDRESS 2

RH_RF95 driver(4, 3); 

RHReliableDatagram manager(driver, CLIENT_ADDRESS);

void setup() 
{
 
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
  driver.setTxPower(23, false);


  driver.setModemConfig(RH_RF95::ModemConfigChoice::Bw500Cr45Sf128);
  driver.setFrequency(915.0);
  manager.setTimeout(2000);
  Serial.println("Setup Complete");

}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
char payload[64];


uint16_t Channel2Read();
uint16_t Channel1Read();

uint16_t ReadSDSCST(uint8_t ChanID);

//uint16_t FuncID;
//uint16_t NNID;
//uint16_t TNID = 0x0001;
//uint16_t ChanID;
uint16_t SensorData;

uint8_t len = sizeof(buf);
uint8_t from;

void loop()
{
  //Serial.println("In the loop");
  StaticJsonBuffer<200> jsonBuffer;
  StaticJsonBuffer<200> jsonBuffer2;
  if (manager.recvfromAck(buf, &len, &from))
  {
    Serial.println("Received Something");
    for(int i = 0; i<len; i++)
      Serial.print((char)buf[i]);
    
    JsonObject& root = jsonBuffer.parseObject(buf);
    uint16_t FNID = root["FNID"];
    Serial.println(FNID);
    uint16_t NNID = root["NNID"];
    uint16_t TNID = root["TNID"];
    uint16_t ChanID = root["ChanID"];
    uint16_t Data = root["Data"];
    /*
    FuncID = buf[0] + buf[1]<<8; 
    NNID = buf[2] + buf[3]<<8;
    TNID = buf[4] + buf[5]<<8;
    ChanID = buf[6] + buf[7]<<8;
    */
    
    switch(FNID)
    {
      case 7211:
        Serial.println("received 7211");
        SensorData = ReadSDSCST(ChanID);
        break;
      /*
      case "7217":
        Payload = WriteSDSCST(ChanID, Data);
        break;
      */
    }

    Serial.println(SensorData);
    JsonObject& root2 = jsonBuffer2.createObject();
    SensorData = (uint16_t)random(73, 88);
    root2["Data"] = SensorData;
    root2["NNID"] = NNID;
    root2["TNID"] = TNID;
    root2["ChanID"] = ChanID;
    root2["FNID"] = FNID;
    root2.printTo(payload, sizeof(payload));
    
      /*
      payload[0] = (uint8_t)(FuncID&0x00FF);   // Set first byte of data to be sent as the FuncID High Byte  
      payload[1] = (uint8_t)((FuncID&0xFF00)>>8);   // Set Second byte to FuncID Low Byte
      payload[2] = (uint8_t)(NNID&0x00FF);;   // Next, the Network Node ID which requested the data
      payload[3] = (uint8_t)((NNID&0xFF00)>>8);
      payload[4] = (uint8_t)(TNID&0x00FF);  // Transducer Node High Byte
      payload[5] = (uint8_t)((TNID&0xFF00)>>8);  // Transducer Node Low Byte
      payload[6] = (uint8_t)(ChanID&0x00FF);  // Channel ID High
      payload[7] = (uint8_t)((ChanID&0xFF00)>>8);   //  Channel ID Low
      payload[8] = (uint8_t)(SensorData&0x00FF); // Just the incremented value
      payload[9] = (uint8_t)((SensorData&0xFF00)>>8);
      */


    //for(int L=0; L<10; L++)
      //Serial.print(payload[L]);
    
    if (!manager.sendtoWait(payload, sizeof(payload), from))
      {
        Serial.println("Send to wait failed");
      }
    }


  delay(50);
}


uint16_t ReadSDSCST(uint16_t ChanID)
{
  switch(ChanID)
  {
    case 1:
      return Channel1Read();
      break;
    case 2:
      return Channel2Read();
      break;
  }
}



uint16_t Channel1Read()
{
  return (uint16_t)random(73, 83);  // Simulate a random temperature and return value
}

uint16_t Channel2Read()
{
  return (uint16_t)random(78, 88);  // Simulate a random temperature and return value
}

