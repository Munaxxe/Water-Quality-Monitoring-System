// Blessed Water Quality Monitoring System
#include <Wire.h>
#include <mDNS.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <WiFi.h>
#include "ThingSpeak.h"
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h >
#include "soc/rtc.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Water Temperature Sensor
const int oneWireBus = 25;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// Initialising Water Temperature sensor
float temperatureC = 0;

// Turbidity Sensor Setup
#define sensor_pin 34
int read_ADC;
int ntu;
bool turbidityStatus;
int tbd = 200;

// PH Sensor Setup
int PH_SENSOR_PIN = 36;
float ph = 7;
float phNA;
float Value = 0;

// Wifi Credentials
const char* ssid = "_";              // Network SSID (name)
const char* password = "munaxxe_";   // Network password

WiFiClient client;

unsigned long myChannelNumber = 3033281;
const char * myWriteAPIKey = "DMGGVNZ2ZHKL2V86";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Pumps Initialisation
int filterPump = 26;
int chlorinationPump = 32;
int bicarbonatePump = 33;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

void setup() {
  Wire.begin();
  Serial.begin(115200);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Initialising");
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Initialising.");
  delay(1000);

  lcd.setCursor(0, 1);
  lcd.print("Please Wait!");
  delay(2000);

  lcd.setCursor(0, 0);
  lcd.print("Initialising..");
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Initialising...");
  delay(1000);

  lcd.setCursor(0, 0);
  lcd.print("Initialising....");
  delay(1000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Water Q-M-System");
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T*C: ");

  lcd.setCursor(0, 1);
  lcd.print("TBD: ");

  lcd.setCursor(7, 1);
  lcd.print("PH: "); // (12, 1)

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialising ThingSpeak

  pinMode(chlorinationPump, OUTPUT);
  pinMode(filterPump, OUTPUT);
  pinMode(bicarbonatePump, OUTPUT);
  pinMode(sensor_pin, INPUT);
  pinMode(PH_SENSOR_PIN, INPUT);

  delay(1000);

  digitalWrite(filterPump, HIGH);
  digitalWrite(bicarbonatePump, HIGH);
  digitalWrite(chlorinationPump, HIGH);
}

void loop() {

  if ((millis() - lastTime) > timerDelay) {

    // Connect or reconnect to WiFi
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect");
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(1000);
      }
      Serial.println("\nConnected.");
    }

    // Start of Temp sensor Code
    sensors.requestTemperatures();
    delay(1000);
    temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    lcd.setCursor(5, 0);
    lcd.print(temperatureC, 1);
    // End of Temp sensor Code

    delay(5000);

    // Start of PH SensorCode
    Value = analogRead(PH_SENSOR_PIN);
    Serial.print(Value);
    Serial.print(" | ");
    float voltage = Value * (3.3 / 4095.0);
    phNA = (3.3 * voltage);

    ph = map(Value, 4095, 3433, 2, 9);

    Serial.println(phNA);
    Serial.println(ph);
    delay(500);
    
    // 
    Serial.print("pH Value: ");
    Serial.println(ph);
    
    lcd.setCursor(11, 1);
    lcd.print(ph, 2);
    delay(1000);

    if (ph < 6) {
      digitalWrite(chlorinationPump, HIGH); // OFF
      digitalWrite(bicarbonatePump, LOW);
      delay(1500);
      digitalWrite(bicarbonatePump, HIGH); 
      delay(4000);
    }

    if (ph >= 6 && ph <= 9) {
      // Water is safe
      digitalWrite(chlorinationPump, HIGH);
      digitalWrite(bicarbonatePump, HIGH);
      delay(100);
    }

    if (ph > 9) {
      digitalWrite(bicarbonatePump, HIGH); // OFF
      digitalWrite(chlorinationPump, LOW);
      delay(1500);
      digitalWrite(chlorinationPump, HIGH);
      delay(4000);
    }
    // End of PH Sensor Code

    // Start of Turbidity Sensor Code
    read_ADC = analogRead(sensor_pin);

    int sf = 2056;
    tbd = map(read_ADC, 0,640, 200, 0);

    Serial.print("read_ADC = ");
    Serial.println(read_ADC);

    Serial.print("tbd = ");
    Serial.println(tbd);
    Serial.println(" ");

    lcd.setCursor(5, 1); //set cursor (colum by row) indexing from 0
    if (tbd < 120) {
      lcd.print("C");

      turbidityStatus == LOW;
      digitalWrite(filterPump, HIGH);      
    }

    if (tbd >= 120) {
      turbidityStatus == HIGH;
      lcd.print("D");

      digitalWrite(filterPump, LOW);
      delay(1000);
      digitalWrite(filterPump, HIGH);
      delay(4000);
    }
    // End of Turbidity Sensor Code

    delay(200);

    // Setting the fields with the values
    ThingSpeak.setField(1, temperatureC);
    ThingSpeak.setField(2, tbd);
    ThingSpeak.setField(3, ph);

    // Write to ThingSpeak
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if (x == 200) {
      Serial.println("Channel update successful.");
    }
    else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}
