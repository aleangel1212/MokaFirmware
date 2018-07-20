#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <ArduinoJson.h>


/*----- Variable Setup -----*/
/* --- Motors --- */
#define MOTOR1 5
#define MOTOR2 4
#define MOTOR3 3

/* --- Pumps --- */
#define PUMP1 8
#define PUMP2 9
#define PUMP3 10

/* --- RFID --- */
//PN532_I2C pn532_i2c(Wire);
//NfcAdapter nfc = NfcAdapter(pn532_i2c);

/* --- Wifi --- */
char ssid[] = "2.4MakeWifiGreatAgain";
char pass[] = "Charge123";

char serverAddress[] = "avecchi.me";
int port = 3000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;


/*----- Helper Functions -----*/
void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

JsonObject& get(String route) {
  StaticJsonBuffer<200> jsonBuffer;
  
  client.get(route);

  String response = client.responseBody();

  JsonObject& root = jsonBuffer.parseObject(response);

  return root;
}

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


/*----- Main Functions -----*/
void setup() {
  Serial.begin(9600);
  while(!Serial);
  
  /*--- Pin Setup ---*/
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(MOTOR3, OUTPUT);

  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(PUMP3, OUTPUT);

  /*--- WiFi Setup ---*/
  while ( status != WL_CONNECTED) {
    Serial.println("Attempting to connect to Network named: " + (String)ssid);
    status = WiFi.begin(ssid, pass);
  }

  printWifiStatus();
  
  /*--- RFID Setup ---*/
  //nfc.begin();
}

void loop() {
  JsonObject& data = get("/coffee/36B6597B");
  int cream = data["cream"];
  Serial.println(cream);
  while(1);
}
