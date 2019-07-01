#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 8;         // change for your board; must be a hardware interrupt pin

String outgoingSensorAddress = "ghdjs7465k";
//String outgoingGatewayAddress = "56789fghij";
String flagNetworkAddress = "0000000000";

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destinationAddress = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

static unsigned long nextSwitchTime = millis();

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

  Serial.println("Initialize MPU6050");

  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // If you want, you can set accelerometer offsets
  // mpu.setAccelOffsetX();
  // mpu.setAccelOffsetY();
  // mpu.setAccelOffsetZ();
  
//  checkSettings();
  
  Serial.println("Waiting for gateway connection...");
}

//void checkSettings() {
//  
//  Serial.println();
//  
//  Serial.print(" * Sleep Mode:            ");
//  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
//  
//  Serial.print(" * Clock Source:          ");
//  switch(mpu.getClockSource())
//  {
//    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
//    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
//    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
//    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
//    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
//    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
//    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
//  }
//  
//  Serial.print(" * Accelerometer:         ");
//  switch(mpu.getRange())
//  {
//    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
//    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
//    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
//    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
//  }  
//
//  Serial.print(" * Accelerometer offsets: ");
//  Serial.print(mpu.getAccelOffsetX());
//  Serial.print(" / ");
//  Serial.print(mpu.getAccelOffsetY());
//  Serial.print(" / ");
//  Serial.println(mpu.getAccelOffsetZ());
//  
//  Serial.println();
//}

void loop() {
  while(flagNetworkAddress == "0000000000"){
    flagNetworkAddress = receiveFlag(LoRa.parsePacket());
  }
  if (millis() - lastSendTime > interval) {
    String senVal;
    if(nextSwitchTime<millis()){
      senVal = String(0);   // send a message
      sendMessage(flagNetworkAddress, senVal);
      nextSwitchTime = millis()+3600000;
    }
    Vector rawAccel1 = mpu.readRawAccel();
    Vector normAccel1 = mpu.readNormalizeAccel();
    delay(100);
    Vector rawAccel2 = mpu.readRawAccel();
    Vector normAccel2 = mpu.readNormalizeAccel();

//    Serial.print("Xraw1 = ");
//    Serial.print(rawAccel1.XAxis);
//    Serial.print(" Yraw1 = ");
//    Serial.print(rawAccel1.YAxis);
//    Serial.print(" Zraw1 = ");
//    Serial.println(rawAccel1.ZAxis);
//    Serial.print("Xraw2 = ");
//    Serial.print(rawAccel2.XAxis);
//    Serial.print(" Yraw2 = ");
//    Serial.print(rawAccel2.YAxis);
//    Serial.print(" Zraw2 = ");
//    Serial.println(rawAccel2.ZAxis);
//    Serial.println();

//  Serial.print(" Xnorm = ");
//  Serial.print(normAccel.XAxis);
//  Serial.print(" Ynorm = ");
//  Serial.print(normAccel.YAxis);
//  Serial.print(" Znorm = ");
//  Serial.println(normAccel.ZAxis);

    int XResult = abs(rawAccel1.XAxis - rawAccel2.XAxis);
    int YResult = abs(rawAccel1.YAxis - rawAccel2.YAxis);
    int ZResult = abs(rawAccel1.ZAxis - rawAccel2.ZAxis);

//    Serial.print("XResult: ");
//    Serial.println(XResult);
//    Serial.print("YResult: ");
//    Serial.println(YResult);
//    Serial.print("ZResult: ");
//    Serial.println(ZResult);
//    Serial.println();

    if(XResult>5000 && YResult<5000 && ZResult<5000){
//        Serial.println("X ALERT");
        senVal = String(100);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult<5000 && YResult>5000 && ZResult<5000){
//        Serial.println("Y ALERT");
        senVal = String(200);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult<5000 && YResult<5000 && ZResult>5000){
//        Serial.println("Z ALERT");
        senVal = String(300);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult>5000 && YResult>5000 && ZResult<5000){
//        Serial.println("X Y ALERT");
        senVal = String(400);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult>5000 && YResult<5000 && ZResult>5000){
//        Serial.println("X Z ALERT");
        senVal = String(500);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult<5000 && YResult>5000 && ZResult>5000){
//        Serial.println("Y Z ALERT");
        senVal = String(600);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
    if(XResult>5000 && YResult>5000 && ZResult>5000){
//        Serial.println("X Y Z ALERT");
        senVal = String(700);   // send a message
        sendMessage(flagNetworkAddress, senVal);
    }
//    Serial.println();
    
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  // parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
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

void sendMessage(String flagNetworkAddress, String sensorValue) {
  
  String outgoingNetworkAddress = "";
  String outgoingGatewayAddress = "";
  String incomingSensorAddress = "";
  
  outgoingNetworkAddress = flagNetworkAddress.substring(0, 10);
  outgoingGatewayAddress = flagNetworkAddress.substring(11, 21);
  incomingSensorAddress = flagNetworkAddress.substring(22, 32);
  
  if(incomingSensorAddress != outgoingSensorAddress){
    Serial.println("Error: This sensor send address is not for me.");
    return;
  }
  
  String outgoing = outgoingSensorAddress + "|" + outgoingNetworkAddress + "|" +  outgoingGatewayAddress + "|" + sensorValue;              // outgoing message

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
    Serial.println("LoRa CLIENT SENDING");
//    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
//    Serial.println("localAddress: 0x" + String(localAddress, HEX));
//    Serial.println("msgCount: " + String(msgCount));
//    Serial.println("outgoing.length: " + String(outgoing.length()));
//    Serial.println("outgoing : " + outgoing);
//    Serial.println("outgoingSensorAddress: " + outgoingSensorAddress);
//    Serial.println("outgoingNetworkAddress: " + outgoingNetworkAddress);
//    Serial.println("outgoingGatewayAddress: " + outgoingGatewayAddress);
//    Serial.println("sensorValue : " + sensorValue);
    Serial.println();
}

void onReceive(int packetSize) {
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
  String uploadInterval = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  incomingNetworkAddress = incoming.substring(0, 10);
  incomingGatewayAddress = incoming.substring(11, 21);
  incomingSensorAddress = incoming.substring(22, 32);
  uploadInterval = incoming.substring(33);
  
  if(uploadInterval == "") return;
  
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
  Serial.println("LoRa CLIENT RECEIVED");
//  Serial.println("localAddress: 0x" + String(localAddress, HEX));
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("incomingMsgId: " + String(incomingMsgId));
//  Serial.println("incomingLength: " + String(incomingLength));
//  Serial.println("incoming: " + incoming);
//  Serial.println("incomingNetworkAddress: " + incomingNetworkAddress);
//  Serial.println("incomingGatewayAddress: " + incomingGatewayAddress);
//  Serial.println("incomingSensorAddress: " + incomingSensorAddress);
//  Serial.println("uploadInterval: " + uploadInterval);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  delay(1000);
}

