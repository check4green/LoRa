#include <SPI.h>
#include <LoRa.h>
#include <JeeLib.h> // Low power functions library
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include "RTClib.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 7

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 8;         // change for your board; must be a hardware interrupt pin

String outgoingSensorAddress = "rece1tare2";
//String outgoingGatewayAddress = "56789fghij";
String flagNetworkAddress = "0000000000";

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device
byte destinationAddress = 0xBB;      // destination to send to
long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

const int transistor = 4;

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
   #define Serial SerialUSB
#endif

RTC_PCF8523 rtc;

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

  pinMode(transistor, OUTPUT);
  
  Serial.println("LoRa init succeeded.");

   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  sensors.begin();
  Serial.println("Waiting for gateway connection...");
}

void loop() {
  while(flagNetworkAddress == "0000000000"){
    flagNetworkAddress = receiveFlag(LoRa.parsePacket());
  }
  digitalWrite(transistor, HIGH);
  if (millis() - lastSendTime > interval) {
    sensors.requestTemperatures(); // Send the command to get temperatures
    int temperature = sensors.getTempCByIndex(0);
    String senVal = String(temperature);   // send a message
    sendMessage(flagNetworkAddress, senVal);
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

  int upInt = uploadInterval.toInt();
  
  //  RTC
    DateTime now = rtc.now();
    int division, dayy, hourr, minutee, secondd;
    secondd = 0;

    if(upInt<60){
       minutee = upInt;
       hourr = 0;
       dayy = 0;
    }

    if(upInt>=60){
       minutee = upInt % 60;
       division = upInt / 60;
       hourr = division % 24;
       dayy = division / 24;
    }

    DateTime future (now + TimeSpan(dayy, hourr, minutee, secondd));

   String futureYearS = String(future.year());
   String futureDayS = String(future.day());
   String futureHourS = String(future.hour());
   String futureMinuteS = String(future.minute());
   String futureDateS = futureYearS + futureDayS + futureHourS + futureMinuteS;
   int futureDateI = futureDateS.toInt();

  int nowYear = now.year();
  int nowMonth = now.month();
  int nowDay = now.day();
  int nowHour = now.hour();
  int nowMinute = now.minute();
  int nowSecond = now.second();

  int futureYear = future.year();
  int futureMonth = future.month();
  int futureDay = future.day();
  int futureHour = future.hour();
  int futureMinute = future.minute();
  int futureSecond = future.second();

//    Serial.println();
//    Serial.println("UploadInterval time: ");
//    Serial.print(dayy);
//    Serial.print('/');
//    Serial.print(hourr);
//    Serial.print('/');
//    Serial.print(minutee);
//    Serial.print('/');
//    Serial.print(secondd);
//    Serial.println();
//    
//    Serial.println();
//    Serial.println("Current time: ");
//    Serial.print(nowYear);
//    Serial.print('/');
//    Serial.print(nowMonth);
//    Serial.print('/');
//    Serial.print(nowDay);
//    Serial.print(' ');
//    Serial.print(nowHour);
//    Serial.print(':');
//    Serial.print(nowMinute);
//    Serial.print(':');
//    Serial.print(nowSecond);
//    Serial.println();
//
//    Serial.println();
//    Serial.println("Upload time: ");
//    Serial.print(futureYear);
//    Serial.print('/');
//    Serial.print(futureMonth);
//    Serial.print('/');
//    Serial.print(futureDay);
//    Serial.print(' ');
//    Serial.print(futureHour);
//    Serial.print(':');
//    Serial.print(futureMinute);
//    Serial.print(':');
//    Serial.print(futureSecond);
//    Serial.println();

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

  Serial.println("Sleeping: " + uploadInterval + " minutes...");
  delay(100);
//  sleeping:
   DateTime noww = rtc.now();
   String nowYearS = String(noww.year());
   String nowDayS = String(noww.day());
   String nowHourS = String(noww.hour());
   String nowMinuteS = String(noww.minute());
   String nowDateS = nowYearS + nowDayS + nowHourS + nowMinuteS;
   int nowDateI = nowDateS.toInt();
   
//  if(nowDateS!=futureDateS){
      LoRa.sleep();
      Sleepy::loseSomeTime(50000);
//      goto sleeping;
//  };
  
  digitalWrite(transistor, HIGH);
  delay(1000);
}

