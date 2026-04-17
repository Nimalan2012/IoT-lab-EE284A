// Define the LED pins
const int LED1_PIN = 15; 
const int LED2_PIN = 32;
const int LED3_PIN = 14;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Set the LED pins as outputs
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  Serial.println("Enter a 3-bit code to control LEDs 1, 2, and 3:");

}

void loop() {
  // put your main code here, to run repeatedly:
  // Read the input string until a newline
  String input = Serial.readStringUntil('\n');

  // Check if there are exactly 3 characters
  if (input.length() == 3) {
    Serial.print("Received code: ");
    Serial.println(input);
    
    // Control LED 1
    if (input.charAt(0) == '1') {
      digitalWrite(LED1_PIN, HIGH);
    } else {
      digitalWrite(LED1_PIN, LOW);
    }

    // Control LED 2
    if (input.charAt(1) == '1') {
      digitalWrite(LED2_PIN, HIGH);
    } else {
      digitalWrite(LED2_PIN, LOW);
    }

    // Control LED 3
    if (input.charAt(2) == '1') {
      digitalWrite(LED3_PIN, HIGH);
    } else {
      digitalWrite(LED3_PIN, LOW);
    }
  }
  else if (input.length() > 0) {
      // Handle invalid input
      Serial.println("Invalid input. Try again.");
    }
}
