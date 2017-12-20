// rf95_reliable_datagram_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging client
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W 

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

//#define CLIENT_ADDRESS 1
uint8_t CLIENT_ADDRESS = 1;

#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
//RH_RF95 driver;
RH_RF95 driver(4, 3); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup() 
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
//  pinMode(4, OUTPUT);
//  digitalWrite(4, HIGH);


  
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  driver.setTxPower(23, false);
  // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
  //  driver.setTxPower(14, true);


  driver.setModemConfig(RH_RF95::ModemConfigChoice::Bw500Cr45Sf128);
  driver.setFrequency(915.0);
  manager.setTimeout(2000);

}

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t isFirstRun = 1;
uint16_t temp;
uint8_t data[8];
uint16_t SensorData = 0;  // Fake Sensor Data
uint8_t TNID0 = 0x00; // Transducer Node ID Low Byte
uint8_t TNID1 = 0x01; // Transducer Node ID High Byte


void loop()
{
  uint8_t len = sizeof(buf);
  uint8_t from;  
  if (manager.recvfromAck(buf, &len, &from))
  {
    for(int i = 0; i<len; i++)
      Serial.println(buf[i]);
    if ((buf[1] == 0x01) && (buf[0] == 0x00)) 
    {
      SensorData++;
      data[0] = 0x00;   // Set first byte of data to be sent as the FuncID High Byte  
      data[1] = 0x01;   // Set Second byte to FuncID Low Byte
      data[2] = from;   // Next, the Network Node ID which requested the data
      data[3] = TNID0;  // Transducer Node High Byte
      data[4] = TNID1;  // Transducer Node Low Byte
      data[5] = buf[5];  // Channel ID High
      data[6] = buf[6];   //  Channel ID Low
      data[7] = SensorData; // Just the incremented value
      Serial.println("Sending to rf95_reliable_datagram_server");
      if (!manager.sendtoWait(data, sizeof(data), from))
      {
        Serial.println("Send to wait failed");
      }
    }
  }



  //data[0] = 'L';
  //temp = analogRead(0);
  //if(temp > 255) temp = 255;
  //data[1] = (uint8_t) temp;
  //Serial.print("Data:");
  //Serial.println(data[1]);
  
  
    
  // Send a message to manager_server
  //if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  //{
    
    
    // Now wait for a reply from the server
  //  uint8_t len = sizeof(buf);
  //  uint8_t from;   
  //  if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
  //  {
  //    Serial.print("got reply from : 0x");
  //    Serial.print(from, HEX);
  //    Serial.print(": ");
  //    Serial.println((char*)buf); 
  //  }
  //  else
  //  {
  //    Serial.println("No reply, is rf95_reliable_datagram_server running?");
  //  }
  //}
  //else
  //  Serial.println("sendtoWait failed");
  delay(50);
}

