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

/* --- Relay ---*/
#define RELAY 999

/* --- Base Water Times --- */
//TODO FOR FULL CAPACITY MEASURE ACTUAL TIMES
#define SIX 2.66
#define EIGHT 3.55
#define TEN  4.44
#define TWELVE 5.32

/* --- One Oz Cream Time --- */
//TODO MEASURE
#define CREAM 10



/* --- RFID --- */
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

/* --- Wifi --- */
char ssid[] = "2.4MakeWifiGreatAgain";
char pass[] = "Charge123";

char serverAddress[] = "avecchi.me";
int port = 3000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;


/*----- Helper Functions -----*/
void turnOnRelay() {
  digitalWrite(RELAY, HIGH)
}

void turnOffRelay() {
  digitalWrite(RELAY, LOW)
}

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

double calcWaterPumpTime(int cream, int size) {

  double waterTime = 0;
  double percentCream = (double)cream;
  percentCream =/ 100;
  if (size == 6)
    waterTime  = SIX*(1-percentCream);
  if (size == 8)
    waterTime  = EIGHT*(1-percentCream);
  if (size == 10)
    waterTime  = TEN*(1-percentCream);
  if (size == 12)
    waterTime  = TWELVE*(1-percentCream);

  return waterTime;

}

double calcCreamPumpTime(int cream, int size) {
  double percentCream = (double)cream;
  percentCream =/ 100;

  double totalCream = percentCream*(double)size;

  return totalCream;

}

int checkPayment(){
  return 1;
}

void defaultCoffee(){
  double waterTime = calcPumppumpTime(0, 8)

  step(1, 2);
  delay(1000);
  pump(1, waterTime);
}

void customCoffee(int type, int size, int cream){
  double waterTime = calcPumppumpTime(cream, size);
  double creamTime = calcCreamPumpTime(cream, size);

  step(type, 2);
  delay(1000);
  pump(1, waterTime);
  delay(10000);
  pump(2, creamTime)
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

  pinMode(RELAY, OUTPUT);

  /*--- WiFi Setup ---*/
  while ( status != WL_CONNECTED) {
    Serial.println("Attempting to connect to Network named: " + (String)ssid);
    status = WiFi.begin(ssid, pass);
  }

  printWifiStatus();

  /*--- RFID Setup ---*/
  nfc.begin();
}

void loop() {

  String userID = "";

  if(nfc.tagPresent()){
    NfcTag tag = nfc.read();
    userID = tag.getUidString();
  }

  if(userID != ""){
    JsonObject& data = get("/coffee/" + userID);
    //JsonObject& data = get("/coffee/36B6597B");
    //TODO how does this find a user but no default coffee?
    if(data != NULL){
      int cream = data["cream"];
      int size  = data["size"];
      //TODO change type of type.
      String type = data["type"];

      if(checkPayment()){
        turnOnRelay();
        delay(10000);
        customCoffee(type, size, cream);
        turnOffRelay();
      }

    }
  }

  delay(10000);
  while(1);
}
