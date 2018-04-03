#include <SPI.h>
#include <LoRa.h>
#include <dht11.h>

dht11 DHT11;
#define DHT11PIN 7

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xBB;     // address of this device
byte destination = 0xFF;      // destination to send to

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa CLIENT");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(866E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }
  Serial.println("LoRa init succeeded.");
}

  // send packet
  void sendMessage(String outgoing) {
    LoRa.beginPacket();                  // start packet
    LoRa.write(destination);              // add destination address
    LoRa.write(localAddress);             // add sender address
    LoRa.write(msgCount);                 // add message ID
    LoRa.write(outgoing.length());        // add payload length
    LoRa.print(outgoing);                 // add payload
    LoRa.endPacket();                     // finish packet and send it
    msgCount++;                           // increment message ID
  }
  
void loop() {
    int chk = DHT11.read(DHT11PIN);
    float temperature = DHT11.temperature;

    String message = String(temperature);   // send a message
    sendMessage(message);
    Serial.println("Sending " + message);
  delay(5000);
}
