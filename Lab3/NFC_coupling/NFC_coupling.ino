#include <Wire.h> //The package for I2C
#include <Adafruit_PN532.h>

// -----------------------------
// PN532
// -----------------------------
// We do not connect these pins but they must be input into the PN532 object
#define PN532_IRQ   -1
#define PN532_RESET -1   

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET, &Wire);


void printUID(const uint8_t *uid, uint8_t uidLength) {
  for (uint8_t i = 0; i < uidLength; i++) {
    if (uid[i] < 0x10) Serial.print("0");
    Serial.print(uid[i], HEX);
    if (i + 1 < uidLength) Serial.print(" ");
  }
}

bool readPageSpan(uint8_t startPage, uint8_t endPage) {
  uint8_t buf[4];

  if (startPage > endPage || endPage > 0x2C) {
    Serial.println("OUT OF RANGE");
    return false;
  }

  for (uint8_t page = startPage; page <= endPage; page++) {
    if (page < 0x10) Serial.print("0");
    Serial.print(page, HEX);
    Serial.print(": ");

    if (nfc.ntag2xx_ReadPage(page, buf)) {
      for (uint8_t i = 0; i < 4; i++) {
        if (buf[i] < 0x10) Serial.print("0");
        Serial.print(buf[i], HEX);
        if (i < 3) Serial.print(" ");
      }
      Serial.println();
    } else {
      Serial.println("READ FAIL");
      return false;
    }
  }

  return true;
}

const uint32_t POLL_TIMEOUT_MS = 250;
const uint32_t INTER_TRIAL_DELAY_MS = 80;
const uint32_t calls = 30;

bool onePoll(uint32_t &ttf_ms) {
uint8_t uid[7];
uint8_t uidLength = 0;
uint32_t t0 = millis();
bool ok = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, POLL_TIMEOUT_MS);
ttf_ms = millis() - t0;
return ok;
}

void setup() {

  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  nfc.begin(); 

  uint32_t versiondata = nfc.getFirmwareVersion();  
  if (!versiondata) {
    Serial.println("ERROR: PN532 not found. Check I2C mode + wiring (and IRQ pin if using Adafruit lib I2C).");
    while (1) delay(10);
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  
  nfc.SAMConfig();

  Serial.println("Waiting for tag...");
}
void loop() {
  if (Serial.available() > 0) {
    char start = Serial.read();
    
    if (start == 's') {
      Serial.println("\n Running 30 Trials.");
      
      int successes = 0;
      uint32_t total_ttf = 0;

      for (int i = 0; i < calls; i++) {
        uint32_t ttf = 0;
        bool success = onePoll(ttf);

        if (success) {
          successes++;
          total_ttf += ttf;
        }
        
        delay(INTER_TRIAL_DELAY_MS);
      }

      Serial.println("\n\n--- Results ---");
      Serial.print("Successful Reads: ");
      Serial.print(successes);
      Serial.println(" / 30");

      float successRate = ((float)successes / calls) * 100.0;
      Serial.print("Success Rate:     ");
      Serial.print(successRate);
      Serial.println("%");

      if (successes > 0) {
        float avg_ttf = (float)total_ttf / successes;
        Serial.print("Average Time:     ");
        Serial.print(avg_ttf);
        Serial.println(" ms");
      } else {
        Serial.println("Average Time:     N/A (0 successes)");
      }
      
      Serial.println("-----------------------------------------");
    }
  }
}
