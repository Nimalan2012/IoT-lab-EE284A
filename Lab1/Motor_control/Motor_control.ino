const int beamPin = 27;
const int servoPin = 13;
const int pwmResolution = 16; // 16-bit resolution

const int pwmFreq = 50;
uint32_t minPulse = 1550;
uint32_t midPulse = 1650;
uint32_t maxPulse = 1750;

float rpmTarget = 60.0;
float rpmMeasured = 0.0;
float currentPulse = 1600.0;
float Kp = 1.0;

int lastState = HIGH;
uint32_t lastBreakTime = 0;

// Convert pulse width in microseconds to duty cycle value
uint32_t dutyFromUs(uint32_t pulseUs) {
const uint32_t maxDuty = (1UL << pwmResolution) - 1;
return (pulseUs * maxDuty) / 20000UL;

}

void setup() {
  Serial.begin(115200);
  pinMode(beamPin, INPUT_PULLUP);

  ledcAttach(servoPin, pwmFreq, pwmResolution);
  delay(100);

  ledcWrite(servoPin, dutyFromUs(maxPulse));
  
  // Serial.print("Testing motor at Pulse Width: ");
  // Serial.println(minPulse);
}

void loop() {
int currentBeamState = digitalRead(beamPin);
  uint32_t currentTime = micros();

  // Detect a state change from high to LOW
  if (lastState == HIGH && currentBeamState == LOW) {
    
    // Calculate the time it took to complete one revolution (Delta T)
    uint32_t deltaT = currentTime - lastBreakTime;
    
// Calculate RPM and execute control loop
    if (deltaT > 0 && lastBreakTime != 0) { 
      // estimate RPM
      rpmMeasured = 60000000.0 / deltaT;
      
      // Compute Error
      float error = rpmTarget - rpmMeasured;

      // Update the PWM pulse width incrementally
      currentPulse = currentPulse + (Kp * error);

      // Clamp the pulse width to the safe range
      if (currentPulse > maxPulse) currentPulse = maxPulse;
      if (currentPulse < minPulse) currentPulse = minPulse;

      // Actuate the servo with the clamped pulse width
      ledcWrite(servoPin, dutyFromUs((uint32_t)currentPulse));
      
      // Print to Serial Plotter
      Serial.print("Target:");
      Serial.print(rpmTarget);
      Serial.print(", Measured:");
      Serial.print(rpmMeasured);
      Serial.print(", Pulse:");
      Serial.println(currentPulse);
    }

    // Reset the timer for the next revolution
    lastBreakTime = currentTime;
  }

  // Save the current state for the next loop iteration
  lastState = currentBeamState;
}

