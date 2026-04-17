#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

const int TMP36_PIN = A2;
const int BUTTON_PIN = BUTTON; 
const int LED_PIN = 13;

const float threshold = 3.0;
const float alpha = 2.0 / (50 + 1.0);

Adafruit_BME680 bme(&Wire); // I2C


float ema_tmp36 = 0;
float bme_temp = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); //ensure LED is off initially

  analogReadResolution(12);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  Serial.println("Press the button to start monitoring.");

  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(10); 
  }
  
  Serial.println("Turning on the system...");

  // Initialise readings
  float voltage = analogReadMilliVolts(TMP36_PIN);
  ema_tmp36 = (voltage - 500.0)/ 10.0;

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  bme.performReading();
  bme_temp = bme.temperature;
}

void loop() {
  // put your main code here, to run repeatedly:

  // Read TMP36 sensor
  float voltage = analogReadMilliVolts(TMP36_PIN);
  float TMP36_temp = (voltage - 500.0)/ 10.0;
  ema_tmp36 = (alpha * TMP36_temp) + ((1.0 - alpha) * ema_tmp36);

  // Read BME688 sensor
  if (bme.performReading()) {
    bme_temp = bme.temperature;
  }

  // Find difference between TMP36 sensor and BME688 sensor
  float difference = abs(ema_tmp36 - bme_temp);

  // Trigger the LED
  if (difference > threshold) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  //Print readings
  Serial.print("TMP36: "); Serial.print(ema_tmp36);
  Serial.print(" C | BME688: "); Serial.print(bme_temp);
  Serial.print(" C | Temp Difference: "); Serial.print(difference);
  Serial.println(" C");
 
  delay(50);
}
