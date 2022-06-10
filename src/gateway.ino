/*
  Master Lora Node
  The IoT Projects

*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "*****";
const char* password = "*****";

// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "PWULOEJXXSF5TP97";
unsigned long lastTime = 0;
// Set timer to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Timer set to 10 seconds (10000)
unsigned long timerDelay = 10000;


#define ss 18  //GPIO 15
#define rst 14  //GPIO 16
#define dio0 26  //GPIO 4

byte MasterNode = 0xFF;
byte Node1 = 0xBB;
byte Node2 = 0xCC;

String SenderNode = "";
String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
String incoming = "";

// Tracks the time since last event fired
unsigned long previousMillis = 0;
unsigned long int previoussecs = 0;
unsigned long int currentsecs = 0;
unsigned long currentMillis = 0;
int interval = 1 ; // updated every 1 second
int Secs = 0;

int temperature;
int humidity;
int soilmoisturepercent;
int soilMoistureValue;

void setup() {
  Serial.begin(115200);                   // initialize serial
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(500);
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  while (!Serial);

  Serial.println("LoRa Master Node");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {


  currentMillis = millis();
  currentsecs = currentMillis / 1000;
  if ((unsigned long)(currentsecs - previoussecs) >= interval) {
    Secs = Secs + 1;
    //Serial.println(Secs);
    if ( Secs >= 11 )
    {
      Secs = 0;
    }
    if ( (Secs >= 1) && (Secs <= 5) )
    {

      String message = "10";
      sendMessage(message, MasterNode, Node1);
    }

    if ( (Secs >= 6 ) && (Secs <= 10))
    {

      String message = "20";
      sendMessage(message, MasterNode, Node2);
    }

    previoussecs = currentsecs;
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());

}


void sendMessage(String outgoing, byte MasterNode, byte otherNode) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(otherNode);              // add destination address
  LoRa.write(MasterNode);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  if ( sender == 0XBB )
    SenderNode = "Node1:";
  if ( sender == 0XCC )
    SenderNode = "Node2:";
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length


  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    //Serial.println("error: message length does not match length");
    ;
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != Node1 && recipient != MasterNode) {
    // Serial.println("This message is not for me.");
    ;
    return;                             // skip rest of function
  }

  if ( sender == 0XCC )
  {
    String t = getValue(incoming, ',', 0); // Temperature
    String h = getValue(incoming, ',', 1); // Humidity

    temperature = t.toInt();
    humidity = h.toInt();
    incoming = "";
    display.print(SenderNode);
    display.print(temperature);
    display.print(humidity);
  }

  if ( sender == 0XBB )
  {

    String v = getValue(incoming, ',', 0); // Soil Moisture Value
    String p = getValue(incoming, ',', 1); // Soil Moisture percentage

    soilMoistureValue = v.toInt();
    soilmoisturepercent = p.toInt();
    incoming = "";
    display.print(SenderNode);
    display.print(soilmoisturepercent);
  }
  display.display();
}
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}