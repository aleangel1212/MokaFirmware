#include <ArduinoHttpClient.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi101.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <ArduinoJson.h>


/*----- Variable Setup -----*/
/* --- Debug --- */
#define DEBUG 1

/* --- Motors --- */
#define MOTOR1 5
#define MOTOR2 4
#define MOTOR3 3

/* --- Pumps --- */
#define PUMP1 8
#define PUMP2 9
#define PUMP3 10

/* --- NeoPixel ---*/
#define NEOPIXEL MOTOR3
#define NUM_PIXELS 16

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL, NEO_GRB + NEO_KHZ800);

#define RED strip.Color(50, 0, 0)
#define GREEN strip.Color(0, 50, 0)
#define BLUE strip.Color(0, 0, 50)
#define PURPLE strip.Color(50, 0, 50)
#define YELLOW strip.Color(50, 50, 0)
#define BLACK strip.Color(0, 0, 0)

/* --- Base Pump Times --- */
#define CREAM_OZ 10
#define WATER_OZ 1.9

/* --- RFID --- */
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

/* --- Wifi --- */
char ssid[] = "Big Veech Mobile";
char pass[] = "1111111111";

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

int getMotor(String type) {
  int motor = 0;
  
  if(type == "regular")
    motor = 1;
  else if(type == "hazelnut")
    motor = 2;

  return motor;
}

double calcCreamPumpTime(int cream) {
  return CREAM_OZ * cream;
}

double calcWaterPumpTime(int cream, int size) {
  return WATER_OZ * (size - cream);
}

void customCoffee(String type, int size, int cream) {
  int motor = getMotor(type);
  double waterTime = calcWaterPumpTime(cream, size);
  double creamTime = calcCreamPumpTime(cream);

  if(DEBUG)
    Serial.println("Type: " + type + "\nSize: " + size + "\nCream: " + cream);

  progress(0, PURPLE);
  
  step(motor, 2);
  progress(20, PURPLE);
  delay(1000);
  
  pump(1, waterTime);
  progress(50, PURPLE);
  delay(5000);

  progress(80, PURPLE);
  pump(2, creamTime);
  progress(100, PURPLE);
  delay(500);

  blink(GREEN, 500);
  delay(1000);
  blink(YELLOW, 1000);
  blink(YELLOW, 1000);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void spinner(int c, int bg, int wait) {
  for(int i = 0; i < strip.numPixels(); i++) {
    colorWipe(bg, 0);
    strip.setPixelColor(((i+3) % strip.numPixels()), c);
    strip.setPixelColor(((i+2) % strip.numPixels()), c);
    strip.setPixelColor(((i+1) % strip.numPixels()), c);
    strip.setPixelColor(((i) % strip.numPixels()), c);
    strip.show();
    delay(wait);
  }
}

void progress(double percent, int c) {
  int pixels = ceil((double)strip.numPixels() * (percent / 100));

  colorWipe(BLACK, 0);
  for(int i = 0; i < pixels; i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}

void blink(int c, int wait) {
  colorWipe(c, 0);
  delay(wait);
  colorWipe(BLACK, 0);
  delay(wait);
  colorWipe(c, 0);
  delay(wait);
  colorWipe(BLACK, 0);
  delay(wait);
}

/*----- Main Functions -----*/
void setup() {
  if(DEBUG) {
    Serial.begin(9600);
    while(!Serial);
  }
  
  /*--- Pin Setup ---*/
  pinMode(MOTOR1, OUTPUT);
  pinMode(MOTOR2, OUTPUT);
  pinMode(MOTOR3, OUTPUT);

  pinMode(PUMP1, OUTPUT);
  pinMode(PUMP2, OUTPUT);
  pinMode(PUMP3, OUTPUT);

  /*--- NeoPixel Setup ---*/
  strip.begin();
  strip.show();

  /*--- WiFi Setup ---*/
  while ( status != WL_CONNECTED) {
    if(DEBUG)
      Serial.println("Attempting to connect to Network named: " + (String)ssid);
      
    status = WiFi.begin(ssid, pass);
    spinner(BLUE, BLACK, 50);
  }

  colorWipe(GREEN, 50);

  if(DEBUG)
    printWifiStatus();
  
  /*--- RFID Setup ---*/
  nfc.begin();
}

void loop() {
  if(DEBUG)
    Serial.println("Waiting for tag...");
    
  spinner(YELLOW, PURPLE, 50);
  
  if (nfc.tagPresent(10)) {
        NfcTag tag = nfc.read();
        tag.print();
        
        colorWipe(GREEN, 50);
        
        JsonObject& data = get("/coffee/" + tag.getUidString());

        if(data["error"]) {
          if(DEBUG);
            Serial.println("No cup found");

          blink(RED, 500);
        }

        else {
          String type = data["type"];
          int size = data["size"];
          int cream = data["cream"];
          
          customCoffee(type, size, cream);
        }
  }

}
