#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define MOTOR1 5
#define MOTOR2 4
#define MOTOR3 3

#define PUMP1 8
#define PUMP2 9
#define PUMP3 10

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

void step(int m, int steps) {
  int motors[] = { MOTOR1, MOTOR2, MOTOR3 };
    
  for(int x = 0; x < 25 * steps; x++) {
    digitalWrite(motors[m-1],HIGH);
    delay(10);
    digitalWrite(motors[m-1],LOW);
    delay(10);
  } 
}

void pump(int p, double seconds) {
  int pumps[] = { PUMP1, PUMP2, PUMP3 };
  
  digitalWrite(pumps[p-1], HIGH);
  delay(1000 * seconds);
  digitalWrite(pumps[p-1], LOW);
}

void setup() {
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(MOTOR3, OUTPUT);

  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(PUMP3, OUTPUT);
  
//  Serial.begin(9600);
//    while(!Serial);
//    Serial.println("NDEF Reader");
//  nfc.begin();
}

void loop() {
  pump(1, 10);
  while(1);
}
