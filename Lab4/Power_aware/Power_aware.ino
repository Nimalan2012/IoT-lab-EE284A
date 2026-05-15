#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_INA219.h>
#include <Wire.h>


#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"

#define fanPin 13
#define pwmFreq 50
#define pwmResolution 8

BLECharacteristic *characteristic;
Adafruit_INA219 ina219;

bool connected = false;
bool restartAdvertising = false;

unsigned long lastAvgTime = 0; 
int sampleCount = 0;           
float powerSum = 0;
float voltageSum = 0;
float currentSum = 0;

char fanState[] = "100%"; 
int dutyCycle = 255; 

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {
    connected = true;
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer*) {
    connected = false;
    restartAdvertising = true;
    Serial.println("Client disconnected");
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  ledcAttach(fanPin, pwmFreq, pwmResolution);
  ledcWrite(fanPin, dutyCycle);


  BLEDevice::init("Nimalan BLE");
  BLEDevice::setPower(ESP_PWR_LVL_P9);
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  BLEService *service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
      CHAR_UUID,
      BLECharacteristic::PROPERTY_NOTIFY
  );

  characteristic->addDescriptor(new BLE2902());
  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("BLE advertising started");
}

void loop() {

  if (restartAdvertising) {
    delay(200);
    BLEDevice::startAdvertising();
    Serial.println("Advertising restarted");
    restartAdvertising = false;
  }
  if (connected) {
    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
    float power_mW = 0;

    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000);

    voltageSum += loadvoltage;
    currentSum += current_mA;
    powerSum += power_mW;
    sampleCount++;

    unsigned long currentMillis = millis();
    
    // Protect the battery to prevent going further below than 3.2V
    if (loadvoltage < 3.25) {
      dutyCycle = 0;
      strcpy(fanState, "OFF");
      ledcWrite(fanPin, dutyCycle);
    } 

    // Power control strategy

    if (currentMillis - lastAvgTime >= 10000) {
      float avgV = voltageSum / sampleCount;
      float avgI = currentSum / sampleCount;
      float avgP = powerSum / sampleCount;

      // Battery is healthy, so maximise the duty cycle
      if (avgV >= 3.5) {
        strcpy(fanState, "100%");
        dutyCycle = 255;
      } 
      // Battery is dropping (3.4V - 3.49V), lower to 50%
      else if (avgV >= 3.4) {
        strcpy(fanState, "50%");
        dutyCycle = 128;
      } 
      // Battery is getting low (3.3V - 3.39V), lower to 25%
      else if (avgV >= 3.3) {
        strcpy(fanState, "25%");
        dutyCycle = 64;
      } 
      // Battery is critical (< 3.3V), Turn OFF to save energy
      else {
        strcpy(fanState, "OFF");
        dutyCycle = 0;
     }
    //  if (avgV >= 3.25) {
    //     
    //     if (avgI > 0.0) {
    //       if (strcmp(fanState, "100%") == 0) {
    //         strcpy(fanState, "50%");
    //         dutyCycle = 128;
    //       } else if (strcmp(fanState, "50%") == 0) {
    //         strcpy(fanState, "25%");
    //         dutyCycle = 64;
    //       } else if (strcmp(fanState, "25%") == 0) {
    //         strcpy(fanState, "OFF");
    //         dutyCycle = 0;
    //       }
    //     }

    //     // Harvesting
    //     else if (avgI < 0) {
    //       if (strcmp(fanState, "OFF") == 0) {
    //         strcpy(fanState, "25%");
    //         dutyCycle = 64;
    //       } else if (strcmp(fanState, "25%") == 0) {
    //         strcpy(fanState, "50%");
    //         dutyCycle = 128;
    //       } else if (strcmp(fanState, "50%") == 0) {
    //         strcpy(fanState, "100%");
    //         dutyCycle = 255;
    //       }
    //     }
    //   }
  

    // Apply the chosen speed to the fan
    ledcWrite(fanPin, dutyCycle);
    
    float current_time = currentMillis / 1000.0;

    // Send 10s Avg data
    char avgData[60];
    snprintf(avgData, sizeof(avgData), "AVG,%.2f,%.2f,%.2f,%s", avgV, avgI, avgP, fanState);
    
    characteristic->setValue(avgData);
    characteristic->notify();

    voltageSum = 0;
    currentSum = 0;
    powerSum = 0;
    sampleCount = 0;
    lastAvgTime = currentMillis;

    delay(1); //small delay to avoid packet interference
  }
  
  char csvdata[50];
  snprintf(csvdata, sizeof(csvdata), "%.2f,%.2f,%.2f,%s", loadvoltage, current_mA, power_mW, fanState);

  characteristic->setValue(csvdata);
  characteristic->notify();
  

  delay(20);
  }
}

