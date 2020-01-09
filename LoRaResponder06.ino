#include <SPI.h>
#include <LoRa.h>

//LoR32u4II 868MHz or 915MHz (black board)
#define SCK     15
#define MISO    14
#define MOSI    16
#define SS      8
#define RST     4
#define DI0     7
#define PABOOST true 
#define BAND    915E6
  
const int SF = 9;
const long bw = 125E3;

int pollerID = 1;
char responderID[] = "012";
char message[40], ackMsg[40], msgDown[40];

void setup() {
  //Serial.begin(9600);   //Remove when on batteries only
  //while (!Serial);
  //sprintf(message, "LoRa responder ID: %s", responderID);
  //Serial.println(message);
  LoRa.setPins(SS,RST,DI0);
  //if (!LoRa.begin(BAND,PABOOST )) {
  //  Serial.println("Starting LoRa failed!");
  //  while (1);
  //}
  LoRa.begin(BAND,PABOOST);
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(bw); 
  // pinMode(1, INPUT);    //Set reed switch pin.  Default behaviour
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received message --> '");      // received a packet
    String msg = "";
    while (LoRa.available()) {        
      msg += ((char)LoRa.read());
    }
    char msgchar[40];
    strcpy(msgchar, msg.c_str());
    Serial.print(msg);
    Serial.println("'");
    //Serial.println(msgchar + (strlen(msgchar) - 8));
    if (!strncmp(msgchar + (strlen(msgchar) - 8), responderID, 3)) {
      sprintf(ackMsg,"Contact made with poller ID : %03d  ", pollerID);
      Serial.println(ackMsg);
      
      LoRa.beginPacket();
      LoRa.write(digitalRead(1));
      LoRa.print(packetSize);
      LoRa.endPacket();
    }  
  }
}
