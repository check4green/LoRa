#include <EEPROM.h>
int magicNumber=57;
int magicAddress=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 

  EEPROM.begin(512);
  Serial.println(EEPROM.read(magicAddress));
  EEPROM.write(magicAddress, magicNumber);
  Serial.println(EEPROM.read(magicAddress));
  EEPROM.end();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
