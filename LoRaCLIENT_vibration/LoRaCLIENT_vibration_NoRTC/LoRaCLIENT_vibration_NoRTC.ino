#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 8;         // change for your board; must be a hardware interrupt pin

String sensorValue;              // outgoing message
String clientAddressForServer;
    
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0x1a;     // address of this device
byte destinationAddress = 0xff;      // destination to send to

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

static unsigned long nextSwitchTime = millis();

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa CLIENT");

  // override the default CS, reset, and IRQ pins (optional)del
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {
    Serial.println("Error: LoRa init failed. Check your connections.");
    while (true);
  }
  
  Serial.println("LoRa init succeeded.");

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
  
  checkSettings();
}

void checkSettings()
{
  Serial.println();
  
  Serial.print(" * Sleep Mode:            ");
  Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");
  
  Serial.print(" * Clock Source:          ");
  switch(mpu.getClockSource())
  {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
  }
  
  Serial.print(" * Accelerometer:         ");
  switch(mpu.getRange())
  {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
  }  

  Serial.print(" * Accelerometer offsets: ");
  Serial.print(mpu.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(mpu.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(mpu.getAccelOffsetZ());
  
  Serial.println();
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
  clientAddressForServer = String(localAddress, HEX);
  clientAddressForServer = "0x" + clientAddressForServer;
  if (millis() - lastSendTime > interval) {
    String sensorValue;
    if(nextSwitchTime<millis()){
      sensorValue = String(0);   // send a message
      sendMessage(sensorValue, clientAddressForServer);
      nextSwitchTime = millis()+3600000;
    }
    Vector rawAccel1 = mpu.readRawAccel();
    Vector normAccel1 = mpu.readNormalizeAccel();
    delay(100);
    Vector rawAccel2 = mpu.readRawAccel();
    Vector normAccel2 = mpu.readNormalizeAccel();

    Serial.print("Xraw1 = ");
    Serial.print(rawAccel1.XAxis);
    Serial.print(" Yraw1 = ");
    Serial.print(rawAccel1.YAxis);
    Serial.print(" Zraw1 = ");
    Serial.println(rawAccel1.ZAxis);
    Serial.print("Xraw2 = ");
    Serial.print(rawAccel2.XAxis);
    Serial.print(" Yraw2 = ");
    Serial.print(rawAccel2.YAxis);
    Serial.print(" Zraw2 = ");
    Serial.println(rawAccel2.ZAxis);
    Serial.println();

//  Serial.print(" Xnorm = ");
//  Serial.print(normAccel.XAxis);
//  Serial.print(" Ynorm = ");
//  Serial.print(normAccel.YAxis);
//  Serial.print(" Znorm = ");
//  Serial.println(normAccel.ZAxis);

    int XResult = abs(rawAccel1.XAxis - rawAccel2.XAxis);
    int YResult = abs(rawAccel1.YAxis - rawAccel2.YAxis);
    int ZResult = abs(rawAccel1.ZAxis - rawAccel2.ZAxis);

    Serial.print("XResult: ");
    Serial.println(XResult);
    Serial.print("YResult: ");
    Serial.println(YResult);
    Serial.print("ZResult: ");
    Serial.println(ZResult);
    Serial.println();

    if(XResult>5000 && YResult<5000 && ZResult<5000){
        Serial.println("X ALERT");
        sensorValue = String(100);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult<5000 && YResult>5000 && ZResult<5000){
        Serial.println("Y ALERT");
        sensorValue = String(200);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult<5000 && YResult<5000 && ZResult>5000){
        Serial.println("Z ALERT");
        sensorValue = String(300);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult>5000 && YResult>5000 && ZResult<5000){
        Serial.println("X Y ALERT");
        sensorValue = String(400);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult>5000 && YResult<5000 && ZResult>5000){
        Serial.println("X Z ALERT");
        sensorValue = String(500);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult<5000 && YResult>5000 && ZResult>5000){
        Serial.println("Y Z ALERT");
        sensorValue = String(600);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    if(XResult>5000 && YResult>5000 && ZResult>5000){
        Serial.println("X Y Z ALERT");
        sensorValue = String(700);   // send a message
        sendMessage(sensorValue, clientAddressForServer);
    }
    Serial.println();

    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }

  onReceive(LoRa.parsePacket());
  
}
