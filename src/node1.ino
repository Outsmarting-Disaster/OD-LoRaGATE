/*
  Lora Node1
  The IoT Projects

*/
#include <SPI.h>              // include libraries
#include <LoRa.h>

#define ss 18
#define rst 4
#define dio0 26

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte MasterNode = 0xFF;
byte Node1 = 0xBB;

int AirValue = 590;   //you need to replace this value with Value_1
int WaterValue = 300;  //you need to replace this value with Value_2
int SensorPin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

String Mymessage = "";

void setup() {
  Serial.begin(115200);                   // initialize serial


  while (!Serial);

  Serial.println("LoRa Node1");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
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
    // Serial.println("error: message length does not match length");
    ;
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != Node1 && recipient != MasterNode) {
    //Serial.println("This message is not for me.");
    ;
    return;                             // skip rest of function
  }
  Serial.println(incoming);
  int Val = incoming.toInt();
  if (Val == 10)
  {
    Mymessage = Mymessage + soilMoistureValue + "," + soilmoisturepercent;
    sendMessage(Mymessage, MasterNode, Node1);
    delay(100);
    Mymessage = "";
  }

}
void sendMessage(String outgoing, byte MasterNode, byte Node1) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(MasterNode);              // add destination address
  LoRa.write(Node1);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}