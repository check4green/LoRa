

void trilateration(float x1,float y1,float r1,float x2,float y2,float r2,float x3,float y3,float r3){
  float A = 2*x2 - 2*x1;
  float B = 2*y2 - 2*y1;
  float C = pow(r1,2) - pow(r2,2) - pow(x1,2) + pow(x2,2) - pow(y1,2) + pow(y2,2);
  float D = 2*x3 - 2*x2;
  float E = 2*y3 - 2*y2;
  float F = pow(r2,2) - pow(r3,2) - pow(x2,2) + pow(x3,2) - pow(y2,2) + pow(y3,2);
  float x = (C*E - F*B) / (E*A - B*D);
  float y = (C*D - A*F) / (B*D - A*E);
  Serial.print("x= ");
  Serial.println(x);
  Serial.print("y= ");
  Serial.println(y);
  Serial.println();
}

void setup() {
  float x1 = 0;
  float y1 = 0;
  float r1 = 29;
  
  float x2 = 16;
  float y2 = 32;
  float r2 = 13;
  
  float x3 = 36;
  float y3 = 0;
  float r3 = 25;
    Serial.begin(9600);
    trilateration(x1, y1, r1, x2, y2, r2, x3, y3, r3);
}

void loop() {
  // put your main code here, to run repeatedly:

}
