#include <SPI.h>
#include <LoRa.h>
#include <JeeLib.h> // Low power functions library
#include <dht11.h>

dht11 DHT11;
#define DHT11PIN 7


const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 8;         // change for your board; must be a hardware interrupt pin

String sensorValue;              // outgoing message
String clientAddressForServer;
    
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0x13;     // address of this device
byte destinationAddress = 0xff;      // destination to send to

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

const int transistor = 4;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa CLIENT");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {
    Serial.println("Error: LoRa init failed. Check your connections.");
    while (true);
  }

  pinMode(transistor, OUTPUT);
  
  Serial.println("LoRa init succeeded.");
}

void onReceive(int packetSize) {
//  LORA
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
    Serial.println("Error: Message length does not match length.");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xff) {
    Serial.println("Error: This message is not for me.");
    return;                             // skip rest of function
  }
  
  int uploadInterval;

  uploadInterval = incoming.toInt();


  // if message is for this device, or broadcast, print details:
  //Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
  Serial.println();
  Serial.println("LoRa CLIENT RECEIVED");
  Serial.println("localAddress: 0x" + String(localAddress, HEX));
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("incomingMsgId: " + String(incomingMsgId));
  Serial.println("incomingLength: " + String(incomingLength));
  Serial.println("incoming: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  
  digitalWrite(transistor, LOW);
  delay(1000);
  String sleeping = String(60000*uploadInterval);
  Serial.println("Sleeping for: " + sleeping);
  for(int i=0; i<uploadInterval; i++){
    LoRa.sleep();
    Sleepy::loseSomeTime(60000);
  };
  digitalWrite(transistor, HIGH);
  delay(1000);
}

  // send packet
  void sendMessage(String sensorValue, String clientAddressForServer) {

    String delimiter = "|";
    String outgoing = sensorValue + delimiter + clientAddressForServer;              // outgoing message

    LoRa.beginPacket();                  // start packet
    LoRa.write(destinationAddress);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);  // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID
    // SERIAL MONITOR
    Serial.println();
    Serial.println("LoRa CLIENT SENDING");
    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
    Serial.println("localAddress: 0x" + String(localAddress, HEX));
    Serial.println("msgCount: " + String(msgCount));
    Serial.println("outgoing.length: " + String(outgoing.length()));
    Serial.println("sensorValue: " + sensorValue);
    Serial.println("clientAddressForServer: " + clientAddressForServer);
    Serial.println("outgoing : " + outgoing);
    Serial.println();
  }
  
void loop() {
  digitalWrite(transistor, HIGH);
  clientAddressForServer = String(localAddress, HEX);
  clientAddressForServer = "0x" + clientAddressForServer;
  if (millis() - lastSendTime > interval) {
    int chk = DHT11.read(DHT11PIN);
    int temperature = DHT11.temperature;
    String sensorValue = String(temperature);   // send a message
    sendMessage(sensorValue, clientAddressForServer);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  onReceive(LoRa.parsePacket());
  
}
