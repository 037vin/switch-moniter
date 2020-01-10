#include <elapsedMillis.h>
#include <SPI.h>
#include <LoRa.h>

//loRa sleep or idle                 2
//Reduce transmission length         3
//Toggle SF - ADR light
//Use frame components more           
//Replace delay()'s                  1

//LoR32u4II 868MHz or 915MHz (black board)
#define SCK     15
#define MISO    14
#define MOSI    16
#define SS      8
#define RST     4
#define DI0     7
#define PABOOST true 
#define BAND    915E6

elapsedMillis timeElapsed;

const int SF = 9;   //check that this is appropriate
const long bw = 125E3;    //check that this is appropriate

//const int pollerID = 1;
const char pollerID[] = "01";
//const int noResponders = 3;
const char responderIDs[] = {
                                      {'1'},
                                      {'2'},
                                      {'3'}
                                      };
char activeResponders[noResponders][];
char activeResponderId;
char ackMessage[40];
char message[40];
int counter = 1;

void setup() {
  Serial.begin(9600);
  while (!Serial);   
  sprintf(message, "LoRa poller ID: %03d", pollerID);
  Serial.println(message);  
  LoRa.setPins(SS,RST,DI0);// set CS, reset, IRQ pin    // override the default CS, reset, and IRQ pins (optional)

  if (!LoRa.begin(BAND,PABOOST)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSpreadingFactor(SF);
  LoRa.setSignalBandwidth(bw);
  Serial.print("Frequency ");
  Serial.print(BAND);
  Serial.print(" Bandwidth ");
  Serial.print(bw);
  Serial.print(" SF ");
  Serial.println(SF);

  //Print responder ID list
  for (int i = 0; strlen(responderIDs[i]) > 0; i++) {
    sprintf(message, "Responder ID's: %s", responderIDs[i]);
    Serial.println(message);
  }
  
  char payload[] = {'P','0','1','>','1','1','<','9','9'};
  //String txt = PID;
  for (int i = 0; i < 9; i++) {
    Serial.print(payload[i]);
  }
  
  
  identifyActiveResponders();
}

void loop() {
   //Use "+" with serial monitor to reset active responder index
   handleSerial();
   
  int activeResponderAck[noResponders];
  
  if (runEvery(5000)) {
    for (int i = 0; strlen(activeResponders[i]) > 0; i++) {

      
      sprintf(message, "Polling active responder %s --> ", activeResponders[i]);
      sendMessage(message);
      //Try 3 times to get a response
      if (!receiveAck(message, i)) {
        activeResponderAck[i] = activeResponderAck[i] + 1;
        Serial.println("Response not received");
      } else {
        //
        activeResponderAck[i] = 0;
        //sprintf(ackMessage, "Responder ID: %s is active", activeResponders[i]);
        //Serial.println(ackMessage);
      }
      if (activeResponderAck[i] > 3) {
        //Some alert to notify monitor
        Serial.print("Lost contact with responder ID: ");
        Serial.println(activeResponders[i]);
        Serial.write(0x07);  // sound alert
        //Remove responder from active reponder array and let continue
        removeActiveResponder(i);
      }  
    }
  }
}

void identifyActiveResponders() {
  //Try three times to get a response from each responder in the list
  for (int i = 0; i < noResponders; i++) {
    int messageAcknowledgedCounter = 0;
    sprintf(message, "Trying responder ID: %s --> ", responderIDs[i]);
    int nackCounter = 0;
    sendMessage(message);
    while (!receiveAck(message,  i) && nackCounter <= 1) {
      Serial.print("No response from ");
      Serial.println(responderIDs[i]);
      // Serial.print(nackCounter);
      LoRa.sleep();
      delay(100);
      sendMessage(message);
      nackCounter++;
    }
    if (nackCounter >= 1) {
      // Serial.println("");
      Serial.println("---- RESPONDER NOT ACTIVE ----");
      delay(100); 
    } else {
      sprintf(ackMessage, "Responder ID %s has responded", responderIDs[i]);
      strcpy(activeResponders[messageAcknowledgedCounter],responderIDs[i]);
      Serial.println(ackMessage);
      messageAcknowledgedCounter++;
    }
    counter++;
  }
}
void removeActiveResponder(int n) {
  if (strlen(activeResponders[n + 1]) > 0) {
    while (strlen(activeResponders[n + 1]) > 0) {
      strcpy(activeResponders[n], activeResponders[n + 1]);
      n++;
    }
  } else {
    strcpy(activeResponders[n],'/0');
  }
}

bool receiveAck(String message, int t) {
  //Serial.println("Waiting for response");
  bool stat = false;
  unsigned long entry = millis();
  while (stat == false && millis() - entry < 2000) {
    if (LoRa.parsePacket()) {
      int bannerState = LoRa.read();
      String ack = "";
      while (LoRa.available()) {
        ack += ((char)LoRa.read());
      }
      int check = 0;
      check = message.length();
      if (ack.toInt() == check) {
        if (bannerState == 0) {
          Serial.println("Banner is up"); 
        }
        if (bannerState == 1) {
          Serial.println("BANNER IS DOWN!!!!!!!!!!!!");  
          Serial.write(0x07);  // sound alert
        }
        Serial.println("RSSI: " + String(LoRa.packetRssi()));
        Serial.println("Snr: " + String(LoRa.packetSnr()));
        stat = true;
      } else {
        Serial.println("Unknown message received");
      }
    }
  }
  return stat;
}

void sendMessage(String message) {
  Serial.print(message);
  // send packet
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
}

void handleSerial() {
 while (Serial.available() > 0) {
   char incomingCharacter = Serial.read();
   switch (incomingCharacter) {
     case '+':
      identifyActiveResponders();
      break;
     case '-':
      break;
    }
 }
}

boolean runEvery(unsigned long interval) {
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
