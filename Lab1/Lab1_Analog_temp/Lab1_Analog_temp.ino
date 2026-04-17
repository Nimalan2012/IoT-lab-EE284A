// Define the ADC pin
const int ADC_PIN = A2;

// Set the reference voltage
const float V_REF = 2.1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogReadResolution(12);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogSetPinAttenuation(ADC_PIN, ADC_6db); 
  delay(50);
  int ADCvalue = analogRead(ADC_PIN);

  // Convert ADC values input into voltage
  float voltage = (ADCvalue/4095.0) * V_REF;
  // Convert voltage into temperature based on TMP36 output charactersistics. The offset of 0.5V is subtracted off.
  float temperature = (voltage - 0.5) * 100;

  // Compare with 23C reference temperature
  String tempref;
  if (temperature > 23.0) {
    tempref = "above";
  } else if (temperature < 23.0) {
    tempref = "below";
  } else {
    tempref = "exactly";
  }

  // Print the data collected and calculated through serial output 
  Serial.print("The raw ADC value is ");
  Serial.print(ADCvalue);
  Serial.print(", which converts to ");
  Serial.print(voltage, 3); 
  Serial.print("V. The temperature is ");
  Serial.print(tempref);
  Serial.print(" 23C, in fact it is ");
  Serial.print(temperature, 2); 
  Serial.println(".");

  delay(1000);
}
