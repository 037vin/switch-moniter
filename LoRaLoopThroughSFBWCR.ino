/*
  LoRa Duplex communication

  Sends a message every half second, and polls continually
  for new incoming messages. Implements a one-byte addressing scheme,
  with 0xFF as the broadcast address.

  Uses readString() from Stream class to read payload. The Stream class'
  timeout may affect other functuons, like the radio's callback. For an

  created 28 April 2017
  by Tom Igoe
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

// uncomment the section corresponding to your board
// BSFrance 2017 contact@bsrance.fr 

//  //LoR32u4 433MHz V1.0 (white board)
//  #define SCK     15
//  #define MISO    14
//  #define MOSI    16
//  #define SS      1
//  #define RST     4
//  #define DI0     7
//  #define BAND    433E6 
//  #define PABOOST true

//  //LoR32u4 433MHz V1.2 (white board)
//  #define SCK     15
//  #define MISO    14
//  #define MOSI    16
//  #define SS      8
//  #define RST     4
//  #define DI0     7
//  #define BAND    433E6 
//  #define PABOOST true 

  //LoR32u4II 868MHz or 915MHz (black board)
  #define SCK     15
  #define MISO    14
  #define MOSI    16
  #define SS      8
  #define RST     4
  #define DI0     7
  #define BAND    915E6
  #define PABOOST true 
  
const int SF = 9;   // default 7
const long bw = 125E3;    // default 125E3
const int CR = 5;       // default 5
char signalBW[9][8] = {           {'7','.','8','E','3','\0'},
                                      {'1','0','.','4','E','3','\0'},
                                      {'1','5','.','6','E','3','\0'},
                                      {'2','0','.','8','E','3','\0'},
                                      {'3','1','.','2','5','E','3','\0'},
                                      {'4','1','.','7','E','3','\0'},
                                      {'6','2','.','5','E','3','\0'},
                                      {'1','2','5','E','3','\0'},
                                      {'2','5','0','E','3','\0'}                      
                                      };

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0x00;     // address of this device
byte destination = 0x00;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 1000;          // interval between sends

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("LoRa Duplex");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(SS,RST,DI0);// set CS, reset, IRQ pin

  if (!LoRa.begin(BAND,PABOOST)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  
  Serial.println("LoRa init succeeded.");
}

void loop() {
  //char message[44];
  int i = 7;
  //while (strlen(signalBW[i]) > 0) {
    Serial.print("SignalBW => ");
    Serial.println(signalBW[i]);
    LoRa.setSignalBandwidth(signalBW[i]);
    int a = 6;
    while (a < 13) {
      Serial.print("Spreading factor => ");
      Serial.println(a);
      LoRa.setSpreadingFactor(a);
      int b = 5;
      while (b < 9) {
       // if (millis() - lastSendTime > interval) {
          Serial.print("Coding rate => ");
          Serial.println(b);
          LoRa.setCodingRate4(b);
          
          //sprintf(message,"");
          //String message = "HeLoRa World! HeLoRa World! HeLoRa World!";   // send a message
          lastSendTime = 0;
          lastSendTime = millis();   
          bool message;
          message = true;
          //message[1] = true;
          sendMessage(message);
          Serial.println(millis() - lastSendTime);
          Serial.println(message);
          //onReceive(LoRa.parsePacket());
          b++;
          //lastSendTime = millis();   
        //}
      }
      a++;
    }
    i++;
  //}
  Serial.println("Loop finished");
  exit(1);
}

void sendMessage(bool outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  //LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                             // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
