#include <SPI.h>
#include <LoRa.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <time.h>
#include <EEPROM.h>

const int csPin = 15;          // LoRa radio chip select
const int resetPin = 4;       // LoRa radio reset
const int irqPin = 5;         // change for your board; must be a hardware interrupt pin

String outgoingGatewayAddress = "56789fghij";
//String outgoingSensorAddress = "01234abcde";
String outgoingNetworkAddressFlag = "";
  
byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destinationAddress = 0xFF;      // destination to send to
long lastSendTime = 0;        // last send time
long lastSendTimeFlag = 0;
int interval = 2000;          // interval between sends

static unsigned long nextSwitchTime = millis();

int timezone = 3 * 3600;
int dst = 0;

String year, mon, mday, hour, minn, sec, tme, tmeRTC;

String ssidpass, ssid, pass;
int address = 0;
char arrayToStore[255];                    // Must be greater than the length of string.

int LED_bluetooth = 2;
int LED_wifi = 0;
int LED_lora = 16;

void setup() {
  pinMode(LED_bluetooth, OUTPUT);
  pinMode(LED_wifi, OUTPUT);
  pinMode(LED_lora, OUTPUT);
  digitalWrite(LED_bluetooth, LOW);
  digitalWrite(LED_wifi, LOW);
  digitalWrite(LED_lora, LOW);
  
  Serial.begin(9600);                   // initialize serial
  while (!Serial);

  Serial.println("LoRa GATEWAY");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {             // initialize ratio at 915 MHz
    Serial.println("Error: LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa init succeeded.");
  digitalWrite(LED_lora, HIGH);
  delay(3000);
  digitalWrite(LED_lora, LOW);
//  EEPROM.begin(512);

  //  BLUETOOTH
  bluetooth:
  ssidpass="0";
  ssid = "0";
  pass = "0";
  
//  if(EEPROM.read(address) != 57){
//    ssidpass = EEPROM.get(address, arrayToStore); 
//  Serial.println();
//  Serial.println(ssidpass);
//  int indexssidpass=0;
//  for(int i=0; String(char(ssidpass[i])) != "|"; i++){
//    indexssidpass = i+1;
//  }
//  ssid = ssidpass.substring(0,indexssidpass);
//  indexssidpass += 1;
//  pass = ssidpass.substring(indexssidpass);
//  Serial.println(ssid);
//  Serial.println(pass);
//
////  WIFI
//  Serial.println();
//  Serial.print("Wifi connecting to ");
//  Serial.println( ssid );
//  WiFi.begin((const char*)ssid.c_str(), (const char*)pass.c_str());
//  Serial.println();
//  Serial.print("Waiting 10sec for WiFi connecting...");
//  }
//   delay(10000);
//  if(WiFi.status() != WL_CONNECTED){
//      digitalWrite(LED_wifi, LOW);
//      Serial.println();
//      Serial.print("Waiting for WiFi SSID and Password...");
//  }
//  while(WiFi.status() != WL_CONNECTED){
//    digitalWrite(LED_bluetooth, HIGH);
//    digitalWrite(LED_wifi, LOW); 
//    if (Serial.available())  /* If data is available on serial port */
//      {
//        ssidpass = Serial.readString();  /* Data received from bluetooth */
//        
//        ssidpass.toCharArray(arrayToStore, ssidpass.length()+1);  // Convert string to array.
//        EEPROM.put(address, arrayToStore);                 // To store data
//        if(ssidpass.length() >=1){
//          goto bluetooth;
//        }
//      }
//  }
//  
//   if(WiFi.status() != WL_CONNECTED){
//      goto bluetooth;
//      digitalWrite(LED_wifi, LOW);
//   }    
//  digitalWrite(LED_bluetooth, LOW);
//  digitalWrite(LED_wifi, HIGH);
//  Serial.println();
//  Serial.println("Wifi Connected Success!");
//  Serial.print("NodeMCU IP Address : ");
//  Serial.println(WiFi.localIP() );
//
//  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
//  Serial.println("\nWaiting for Internet time.");
//
//  while(!time(nullptr)){
//     Serial.print("*");
//     delay(1000);
//  }
//  Serial.println("\nTime response....OK");   
//  Serial.println();
//
//  EEPROM.end();
//
}

void loop() {
//  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
//    digitalWrite(LED_wifi, HIGH);
//  }else{
//    digitalWrite(LED_wifi, LOW);
//   }
  // try to parse packet
  if (millis() - lastSendTimeFlag > interval) {
    if(nextSwitchTime<millis()){
      outgoingNetworkAddressFlag = sendFlag();
      nextSwitchTime = millis()+60000;
    }
      lastSendTimeFlag = millis();            // timestamp the message
      interval = random(2000) + 1000;    // 2-3 seconds
  }
  onReceive(LoRa.parsePacket(), outgoingNetworkAddressFlag);
}

  String sendFlag(){
    String outgoingNetworkAddress = "38465thdku";
    String outgoingSensorList[] = {"oftmdjrtgl", "4637281930", "94jfc84jg8", "ghdj475843", "jgfld75tn9", "01234abcde", "ggth44487h", "44444ggggg", "ff4gtdtgd3", "ffff8888r5"};
    String flag = "";
    int outgoingSensorListContor = sizeof(outgoingSensorList)/12;
    String numberOfDevices = String(outgoingSensorListContor);
    for(int i=0; i<outgoingSensorListContor; i++){
      flag = outgoingNetworkAddress + "|" + outgoingGatewayAddress + "|" + outgoingSensorList[i];              // flag message

    LoRa.beginPacket();                  // start packet
      if(LoRa.beginPacket()){
        digitalWrite(LED_lora, HIGH);  
      }
    LoRa.write(destinationAddress);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(flag.length());        // add payload length
    LoRa.print(flag);  // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID 

    // SERIAL MONITOR
    Serial.println();
    Serial.println("LoRa GATEWAY SENDING flag");
//    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
//    Serial.println("localAddress: 0x" + String(localAddress, HEX));
//    Serial.println("msgCount: " + String(msgCount));
//    Serial.println("flag.length: " + String(flag.length()));
//    Serial.println("numberOfDevices: " + numberOfDevices);
//    Serial.println("flag: " + flag);
//    Serial.println("outgoingNetworkAddress: " + outgoingNetworkAddress);
//    Serial.println("outgoingGatewayAddress: " + outgoingGatewayAddress);
//    Serial.println("outgoingSensorList: " + outgoingSensorList[i]);
    Serial.println();
    digitalWrite(LED_lora, LOW); 
    }

    return outgoingNetworkAddress;
}

int sendMessage(String outgoingSensorAddress, String outgoingNetworkAddress) {
  
String upInt, gA, cA;

////WIFI
//  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
//    digitalWrite(LED_wifi, HIGH);
//    HTTPClient http;    //Declare object of class HTTPClient
////    http.begin("http://swiss-iot.azurewebsites.net/api/sensors/address/" + outgoingGatewayAddress + "/" + outgoingSensorAddress);  //Specify request destination
//    http.begin("http://192.168.0.18:32333/api/sensors/address/" + outgoingGatewayAddress + "/" + outgoingSensorAddress);  //Specify request destination
//    http.addHeader("Authorization", "Basic Y3JnQHlhaG9vLmNvbTpwYWNhbGE=", true, true);
//    int httpCodeGET = http.GET();     //Send the request
//    if (httpCodeGET > 0) { //Check the returning code
//      
//      StaticJsonBuffer<500> jsonBuffer;
//      JsonObject& root = jsonBuffer.parseObject(http.getString());
//      if (!root.success()) {
//        Serial.println("Error: Parse object failed.");
//        return 000;
//      }
//      
//      const char* uploadInterval = root["uploadInterval"];
//      const char* gatewayAddress = root["gatewayAddress"];
//      const char* clientAddress = root["clientAddress"];
//      upInt = String(uploadInterval);
//      gA = String(gatewayAddress);
//      cA = String(clientAddress);
//      Serial.println();
//      Serial.println("SERVER GET RESPONSE");
//      Serial.println("Response: " + String(httpCodeGET));   //Print HTTP return code
//      Serial.println("uploadInterval: " + upInt);   
//      Serial.println("gatewayAddress: " + gA);   
//      Serial.println("clientAddress: " + cA);
//      Serial.println();
//    }
//
//    http.end();   //Close connection
// 
//  } else {
//    Serial.println("Error: WiFi connection.");
//    digitalWrite(LED_wifi, LOW);
//  }
  upInt = 1;
  String outgoing = outgoingNetworkAddress + "|" + outgoingGatewayAddress + "|" + outgoingSensorAddress + "|" + upInt;              // outgoing message
  
  LoRa.beginPacket();                  // start packet
    if(LoRa.beginPacket()){
       digitalWrite(LED_lora, HIGH);  
    }
    LoRa.write(destinationAddress);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);  // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID 

    // SERIAL MONITOR
    Serial.println();
    Serial.println("LoRa GATEWAY SENDING");
//    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
//    Serial.println("localAddress: 0x" + String(localAddress, HEX));
//    Serial.println("msgCount: " + String(msgCount));
//    Serial.println("outgoing.length: " + String(outgoing.length()));
//    Serial.println("outgoing: " + outgoing);
//    Serial.println("outgoingNetworkAddress: " + outgoingNetworkAddress);
//    Serial.println("outgoingGatewayAddress: " + outgoingGatewayAddress);
//    Serial.println("outgoingSensorAddress: " + outgoingSensorAddress);
//    Serial.println("uploadInterval: " + upInt);
    Serial.println();
    digitalWrite(LED_lora, LOW);
    return 300;
    
}

void onReceive(int packetSize, String outgoingNetworkAddress) {
  //  LORA
  if (packetSize == 0) {
    return;
  }else{
      digitalWrite(LED_lora, HIGH);
    }          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";
  String incomingSensorAddress = "";
  String incomingGatewayAddress = "";
  String incomingNetworkAddress = "";
  String sensorValue = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  incomingSensorAddress = incoming.substring(0, 10);
  incomingNetworkAddress = incoming.substring(11, 21);
  incomingGatewayAddress = incoming.substring(22, 32);
  sensorValue = incoming.substring(33);

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("Error: message length does not match length");
    digitalWrite(LED_lora, LOW); 
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xBB) {
    Serial.println("Error: This hexa address is not for me.");
    digitalWrite(LED_lora, LOW); 
    return;                             // skip rest of function
  }

  if (incomingGatewayAddress != outgoingGatewayAddress) {
    Serial.println("Error: This gateway address is not for me.");
    digitalWrite(LED_lora, LOW); 
    return;                             // skip rest of function
  }
  if (incomingNetworkAddress != outgoingNetworkAddress) {
    Serial.println("Error: This network address is not for me.");
    digitalWrite(LED_lora, LOW); 
    return;                             // skip rest of function
  }

  time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);

    year = String(p_tm->tm_year + 1900);
    
    if(p_tm->tm_mday < 10){
      mday = String(p_tm->tm_mday);
      mday = "0" + mday;
    }else{
      mday = String(p_tm->tm_mday);
     }
    if(p_tm->tm_mon < 9){
      mon = String(p_tm->tm_mon + 1);
      mon = "0" + mon;
    }else{
      mon = String(p_tm->tm_mon + 1);
     }
    if(p_tm->tm_hour < 10){
      hour = String(p_tm->tm_hour);
      hour = "0" + hour;
    }else{
      hour = String(p_tm->tm_hour);
     }
    if(p_tm->tm_min < 10){
      minn = String(p_tm->tm_min);
      minn = "0" + minn;
    }else{
      minn = String(p_tm->tm_min);
     }
    if(p_tm->tm_sec < 10){
      sec = String(p_tm->tm_sec);
      sec = "0" + sec;
    }else{
      sec = String(p_tm->tm_sec);
     }
    
    tme = year + "-" + mon + "-" + mday + "T" + hour + ":" + minn + ":" + sec + "+02:00";
//    tmeRTC = "|" + year + "/" + mon + "/" + mday + "/" + hour + "/" + minn + "/" + sec;

  // if message is for this device, or broadcast, print details:
  //Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));

  // if message is for this device, or broadcast, print details:
  Serial.println();
  Serial.println("LoRa GATEWAY RECEIVED");
//  Serial.println("localAddress: 0x" + String(localAddress, HEX));
//  Serial.println("Received from: 0x" + String(sender, HEX));
//  Serial.println("Sent to: 0x" + String(recipient, HEX));
//  Serial.println("incomingMsgId: " + String(incomingMsgId));
//  Serial.println("incoming.length: " + String(incomingLength));
//  Serial.println("incoming: " + incoming);
//  Serial.println("incomingSensorAddress: " + incomingSensorAddress);
//  Serial.println("incomingNetworkAddress: " + incomingNetworkAddress);
//  Serial.println("incomingGatewayAddress: " + incomingGatewayAddress);
//  Serial.println("sensorValue: " + sensorValue);
//  Serial.println("RSSI: " + String(LoRa.packetRssi()));
//  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  
    int gatewayAddressVerify;
    if (millis() - lastSendTime > interval) {
      gatewayAddressVerify = sendMessage(incomingSensorAddress, outgoingNetworkAddress);
      lastSendTime = millis();            // timestamp the message
      interval = random(2000) + 1000;    // 2-3 seconds
    }
    digitalWrite(LED_lora, LOW);
////    //WIFI
//  if(gatewayAddressVerify == 300){   
//  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
//    digitalWrite(LED_wifi, HIGH);
// 
//    StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
//    JsonObject& JSONencoder = JSONbuffer.createObject(); 
//
//    JSONencoder["sensorGatewayAddress"] = outgoingGatewayAddress;
//    JSONencoder["sensorClientAddress"] = incomingSensorAddress;
//    JSONencoder["value"] = sensorValue;
//    JSONencoder["readingDate"] = tme;
// 
//    char JSONmessageBuffer[300];
//    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
// 
//    HTTPClient http;    //Declare object of class HTTPClient
// 
////    http.begin("http://swiss-iot.azurewebsites.net/api/readings/address");      //Specify request destination
//    http.begin("http://192.168.0.18:32333/api/readings/address");      //Specify request destination
//   
//    http.addHeader("Content-Type", "application/json");  //Specify content-type header
// 
//    int httpCodePOST = http.POST(JSONmessageBuffer);   //Send the request
//    if (httpCodePOST > 0) { //Check the returning code
//      String payload = http.getString();      
//      //Get the response payload
//      Serial.println();
//      Serial.println("SERVER POST RESPONSE");
//      Serial.println("Time: " + tme);
//      Serial.println("JSONmessageBuffer: " + String(JSONmessageBuffer));
//      Serial.println("response: " + String(httpCodePOST));   //Print HTTP return code
//      Serial.println("payload: " + payload);    //Print request response payload
//      Serial.println();
//    }
//    http.end();  //Close connection
//    
//  } else {
//    Serial.println("Error: WiFi connection.");
//    digitalWrite(LED_wifi, LOW);
//  }
//  }
  
}

