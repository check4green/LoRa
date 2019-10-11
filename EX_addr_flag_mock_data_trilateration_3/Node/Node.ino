#include <SPI.h>
#include <LoRa.h>

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 8;         // change for your board; must be a hardware interrupt pin

String outgoingSensorAddress = "e1e2e3e4e5";
//String outgoingGatewayAddress = "56789fghij";
String flagAddress = "0000000000";

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destinationAddress = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

String localizationFlag = "";

void setup() {
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("LoRa CLIENT");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {             // initialize ratio at 915 MHz
    Serial.println("Error: LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }
  
  Serial.println("LoRa init succeeded.");

  Serial.println("Waiting for gateway connection...");
}

void loop() {
  while(flagAddress == "0000000000"){
    flagAddress = receiveFlag(LoRa.parsePacket());
  }
    // parse for a packet, and call onReceive with the result:
   onReceive(LoRa.parsePacket(), flagAddress);
}

String receiveFlag(int packetSize){
  
  if (packetSize == 0) return "0000000000";          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String flag = "";
  String incomingNetworkAddress = "";
  String incomingGatewayAddress = "";
  String incomingSensorAddress = "";
  
  while (LoRa.available()) {
    flag += (char)LoRa.read();
  }

  incomingNetworkAddress = flag.substring(0, 10);
  incomingGatewayAddress = flag.substring(11, 21);
  incomingSensorAddress = flag.substring(22, 32);
  
  if (incomingLength != flag.length()) {   // check length for error
    Serial.println("Error: Message flag length does not match length.");
    return "0000000000";                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("Error: This hexa flag address is not for me.");
    return "0000000000";                             // skip rest of function
  }
  if (incomingSensorAddress != outgoingSensorAddress) {
    Serial.println("Error: This sensor flag address is not for me.");
    return "0000000000";                             // skip rest of function
  }

  Serial.println();
  Serial.println("LoRa CLIENT RECEIVED flag");
//  Serial.println("localAddress: 0x" + String(localAddress, HEX));
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("incomingMsgId: " + String(incomingMsgId));
//  Serial.println("incomingLength: " + String(incomingLength));
//  Serial.println("incoming: " + flag);
//  Serial.println("incomingNetworkAddress: " + incomingNetworkAddress);
//  Serial.println("incomingGatewayAddress: " + incomingGatewayAddress);
//  Serial.println("incomingSensorAddress: " + incomingSensorAddress);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  Serial.println("Gateway connected.");
  
  return flag;
}

void sendMessage(String flagAddress) {
  
  String outgoingNetworkAddress = "";
  String outgoingGatewayAddress = "";
  String incomingSensorAddress = "";
  String outgoingSensorId = "";
  
  outgoingNetworkAddress = flagAddress.substring(0, 10);
  outgoingGatewayAddress = flagAddress.substring(11, 21);
  incomingSensorAddress = flagAddress.substring(22, 32);
  
  if(incomingSensorAddress != outgoingSensorAddress){
    Serial.println("Error: This sensor send address is not for me.");
    return;
  }

  long randomL = random(1000, 9999);
  int randomN = (int)randomL;
  outgoingSensorId = String(randomN);
  
  String outgoing = outgoingSensorAddress + "|" + outgoingNetworkAddress + "|" +  outgoingGatewayAddress + "|" + outgoingSensorId;              // outgoing message

  LoRa.beginPacket();                   // start packet
  LoRa.write(destinationAddress);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID

  // SERIAL MONITOR
    Serial.println();
    Serial.println("LoRa CLIENT SENDING localization");
//    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
//    Serial.println("localAddress: 0x" + String(localAddress, HEX));
//    Serial.println("msgCount: " + String(msgCount));
//    Serial.println("outgoing.length: " + String(outgoing.length()));
//    Serial.println("outgoing : " + outgoing);
//    Serial.println("outgoingSensorAddress: " + outgoingSensorAddress);
//    Serial.println("outgoingNetworkAddress: " + outgoingNetworkAddress);
//    Serial.println("outgoingGatewayAddress: " + outgoingGatewayAddress);
    Serial.println();
}

void onReceive(int packetSize, String flagAddress) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";
  String incomingSensorAddress = "";
  String incomingGatewayAddress = "";
  String incomingNetworkAddress = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  incomingNetworkAddress = incoming.substring(0, 10);
  incomingGatewayAddress = incoming.substring(11, 21);
  incomingSensorAddress = incoming.substring(22, 32);
  
  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("Error: Message length does not match length");
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("Error: This hexa address is not for me.");
    return;                             // skip rest of function
  }

  if (incomingSensorAddress != outgoingSensorAddress) {
    Serial.println("Error: This sensor address is not for me.");
    return;                             // skip rest of function
  }
  
  Serial.println();
  Serial.println("LoRa CLIENT RECEIVED localization");
//  Serial.println("localAddress: 0x" + String(localAddress, HEX));
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("incomingMsgId: " + String(incomingMsgId));
//  Serial.println("incomingLength: " + String(incomingLength));
//  Serial.println("incoming: " + incoming);
//  Serial.println("incomingNetworkAddress: " + incomingNetworkAddress);
//  Serial.println("incomingGatewayAddress: " + incomingGatewayAddress);
//  Serial.println("incomingSensorAddress: " + incomingSensorAddress);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  if (millis() - lastSendTime > interval) {
      sendMessage(flagAddress);
      lastSendTime = millis();            // timestamp the message
      interval = random(2000) + 1000;    // 2-3 seconds
    }
}

