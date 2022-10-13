
// Setting pin constants for easier use/understanding.
const uint8_t BTN_PIN = 2;
const uint8_t LED_PIN = 13;

// Initialising variables to low.
uint8_t buttonPrevState = LOW;
uint8_t ledState = LOW;

// Setting up the input/output of pins and 
// starting serial communication.
void setup()
{
  // Setting button pin as an input.
  pinMode(BTN_PIN, INPUT_PULLUP);
  // Setting LED pin as an output.
  pinMode(LED_PIN, OUTPUT);
  // Starting serial communication at specified baud rate.
  Serial.begin(9600);
}

void loop()
{
  // Setting variable to value read in from the button.
  uint8_t buttonState = digitalRead(BTN_PIN);
  
  // print various states to the serial monitor.
  Serial.print(buttonState);
  Serial.print(buttonPrevState);
  Serial.print(ledState);
  Serial.println("");
  
  
  // Change LED state if 'buttonstate' differs from 'button PrevState' 
  // and write to the LED. Update previous button state with current.
  if(buttonState != buttonPrevState)
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
  
  buttonPrevState = buttonState;
    
  // Add a 500 ms delay.
  delay(500);
}