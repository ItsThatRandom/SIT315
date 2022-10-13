
// Setting pin constants for easier use/understanding. 
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;

// Setting up the input/output of pins and starting serial communication.
void setup()
{
  // Setting button pin as an input. 
  pinMode(BTN_PIN, INPUT_PULLUP); 
  // Setting LED pin as an output. 
  pinMode(LED_PIN, OUTPUT); 
  // Starting serial communication at specified baud rate. 
  Serial.begin(9600); 
  // Add interrupts for change in sensor values. 
  attachInterrupt (digitalPinToInterrupt (BTN_PIN), toggleLed, CHANGE);
}

void loop()
{
  // Delay can be removed or replace with desired task. 
  delay(500);
}

  void toggleLed() {
// Setting variable to value read in from the button. 
    uint8_t buttonState = digitalRead(BTN_PIN);
    
// Print state to the serial monitor. 
    Serial.print (buttonState);
    
// Write to LED pin to match the buttons state. 
    digitalWrite(LED_PIN, buttonState);

}

