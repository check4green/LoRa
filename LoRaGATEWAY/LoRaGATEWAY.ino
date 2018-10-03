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

String gatewayAddressForServer;
byte localAddress = 0xff;     // address of this device
byte destinationAddress = 0x00;      // destination to send to

long lastSendTime = 0;        // last send time
int interval = 2000;          // interval between sends

int timezone = 3 * 3600;
int dst = 0;

String year, mon, mday, hour, minn, sec, tme;

byte msgCount = 0;            // count of outgoing messages

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
  
//  LORA
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa GATEWAY");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {
    Serial.println("Error: LoRa init failed. Check your connections.");
    digitalWrite(LED_lora, LOW);
    while (true);
  }
  Serial.println("LoRa init succeeded.");
  digitalWrite(LED_lora, HIGH);
  delay(3000);
  digitalWrite(LED_lora, LOW);
  EEPROM.begin(512);

//  BLUETOOTH
  bluetooth:
  ssidpass="0";
  ssid = "0";
  pass = "0";

  ssidpass = EEPROM.get(address, arrayToStore);
  Serial.println();
  Serial.println(ssidpass);
  int indexssidpass=0;
  for(int i=0; String(char(ssidpass[i])) != "|"; i++){
    indexssidpass = i+1;
  }
  ssid = ssidpass.substring(0,indexssidpass);
  indexssidpass += 1;
  pass = ssidpass.substring(indexssidpass);
  Serial.println(ssid);
  Serial.println(pass);

//  WIFI
  Serial.println();
  Serial.print("Wifi connecting to ");
  Serial.println( ssid );
  WiFi.begin((const char*)ssid.c_str(), (const char*)pass.c_str());
  Serial.println();
  Serial.print("Waiting 10sec for WiFi connecting...");
   delay(10000);
  if(WiFi.status() != WL_CONNECTED){
      digitalWrite(LED_wifi, LOW);
      Serial.println();
      Serial.print("Waiting for WiFi SSID and Password");
  }
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(LED_bluetooth, HIGH);
    digitalWrite(LED_wifi, LOW);
    delay(500);
    Serial.print(".");  
    if (Serial.available())  /* If data is available on serial port */
      {
        ssidpass = Serial.readString();  /* Data received from bluetooth */
        
        ssidpass.toCharArray(arrayToStore, ssidpass.length()+1);  // Convert string to array.
        EEPROM.put(address, arrayToStore);                 // To store data
        if(ssidpass.length() >=1){
          goto bluetooth;
        }
      }
  }
  
   if(WiFi.status() != WL_CONNECTED){
      goto bluetooth;
      digitalWrite(LED_wifi, LOW);
   }    
  digitalWrite(LED_bluetooth, LOW);
  digitalWrite(LED_wifi, HIGH);
  Serial.println();
  Serial.println("Wifi Connected Success!");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP() );

  configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time.");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK");   
  Serial.println();

  EEPROM.end();

}

  // send packet
  int sendMessage(String sA, byte iCA) {
    String upInt, gA, cA, lA, iCAc;
    
    lA = String(localAddress, HEX);
    lA = "0x"+lA;
    iCAc = String(iCA, HEX);
    iCAc = "0x"+iCAc;
    
  //WIFI
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    digitalWrite(LED_wifi, HIGH);
    HTTPClient http;    //Declare object of class HTTPClient
    http.begin("http://swiss-iot.azurewebsites.net/api/sensors/address/" + lA + "/" + iCAc);  //Specify request destination
//    http.begin("http://192.168.0.18:32333/api/sensors/address/" + lA + "/" + iCAc);  //Specify request destination
    http.addHeader("Authorization", "Basic Y3JnQHlhaG9vLmNvbTpwYWNhbGE=", true, true);
    int httpCodeGET = http.GET();     //Send the request
    if (httpCodeGET > 0) { //Check the returning code
      
      StaticJsonBuffer<400> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(http.getString());
      if (!root.success()) {
        Serial.println("Error: Parse object failed.");
        return 000;
      }
      
      const char* uploadInterval = root["uploadInterval"];
      const char* gatewayAddress = root["gatewayAddress"];
      const char* clientAddress = root["clientAddress"];
      upInt = String(uploadInterval);
      gA = String(gatewayAddress);
      cA = String(clientAddress);
      Serial.println();
      Serial.println("SERVER GET RESPONSE");
      Serial.println("Response: " + String(httpCodeGET));   //Print HTTP return code
      Serial.println("uploadInterval: " + upInt);   
      Serial.println("gatewayAddress: " + gA);   
      Serial.println("clientAddress: " + cA);
      Serial.println();
    }
 
    http.end();   //Close connection
 
  } else {
    Serial.println("Error: WiFi connection.");
    digitalWrite(LED_wifi, LOW);
  }
  
    //LORA
    if(lA != gA){
      Serial.println("Error: gatewayAddress: " + gA + " is false."); 
      return 100;  
    }
    if(iCAc != cA){
      Serial.println("Error: clientAddress: " + cA + " is false."); 
      return 200;  
    }
    
    String outgoing = upInt;              // outgoing message
    
    destinationAddress = iCA;
    
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
    Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
    Serial.println("localAddress: 0x" + String(localAddress, HEX));
    Serial.println("msgCount: " + String(msgCount));
    Serial.println("outgoing.length: " + String(outgoing.length()));
    Serial.println("outgoing: " + outgoing);
    Serial.println();
    digitalWrite(LED_lora, LOW);
    return 300;
  }

void onReceive(int packetSize) {
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

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("Error: Message length does not match length.");
    digitalWrite(LED_lora, LOW); 
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0x00) {
    Serial.println("Error: This message is not for me.");
    return;                             // skip rest of function
  }
  
  //Split incomming message
  String sensorValue, sensorAddress;
  int indexSensorValue=0;
  for(int i=0; String(char(incoming[i])) != "|"; i++){
    indexSensorValue = i+1;
  }
  sensorValue = incoming.substring(0,indexSensorValue);
  indexSensorValue += 1;
  sensorAddress = incoming.substring(indexSensorValue);


  // if message is for this device, or broadcast, print details:
  //Serial.println("destinationAddress: 0x" + String(destinationAddress, HEX));
  Serial.println();
  Serial.println("LoRa GATEWAY RECEIVED");
  Serial.println("localAddress: 0x" + String(localAddress, HEX));
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("incomingMsgId: " + String(incomingMsgId));
  Serial.println("incoming.length: " + String(incomingLength));
  Serial.println("incoming: " + incoming);
  Serial.println("sensorValue: " + sensorValue);
  Serial.println("sensorAddress: " + sensorAddress);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  int gatewayAddressVerify;
  
  if (millis() - lastSendTime > interval) {
    gatewayAddressVerify = sendMessage(sensorAddress, sender);
    lastSendTime = millis();            // timestamp the message
    interval = random(2000) + 1000;    // 2-3 seconds
  }
    digitalWrite(LED_lora, LOW);
    
  //WIFI
  if(gatewayAddressVerify == 300){   
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    digitalWrite(LED_wifi, HIGH);
    
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
 
    StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
    JsonObject& JSONencoder = JSONbuffer.createObject(); 

    gatewayAddressForServer = String(localAddress, HEX);
    gatewayAddressForServer = "0x" + gatewayAddressForServer;

    JSONencoder["sensorGatewayAddress"] = gatewayAddressForServer;
    JSONencoder["sensorClientAddress"] = sensorAddress;
    JSONencoder["value"] = sensorValue;
    JSONencoder["readingDate"] = tme;
 
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
 
    HTTPClient http;    //Declare object of class HTTPClient
 
    http.begin("http://swiss-iot.azurewebsites.net/api/readings/address");      //Specify request destination
//    http.begin("http://192.168.0.18:32333/api/readings/address");      //Specify request destination
   
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
 
    int httpCodePOST = http.POST(JSONmessageBuffer);   //Send the request
    if (httpCodePOST > 0) { //Check the returning code
      String payload = http.getString();      
      //Get the response payload
      Serial.println();
      Serial.println("SERVER POST RESPONSE");
      Serial.println("Time: " + tme);
      Serial.println("JSONmessageBuffer: " + String(JSONmessageBuffer));
      Serial.println("response: " + String(httpCodePOST));   //Print HTTP return code
      Serial.println("payload: " + payload);    //Print request response payload
      Serial.println();
    }
    http.end();  //Close connection
    
  } else {
    Serial.println("Error: WiFi connection.");
    digitalWrite(LED_wifi, LOW);
  }
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    digitalWrite(LED_wifi, HIGH);
  }else{
    digitalWrite(LED_wifi, LOW);
   }
  // try to parse packet
  onReceive(LoRa.parsePacket());
}
