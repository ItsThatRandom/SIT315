// Task 1.3C SIT315, by Ryan Waites

// Digital pin devices
int movementSensor = 2;
int buttonSensor = 3;
int greenLed = 4;
int redLed = 5;

void setup()
{
  Serial.begin(9600);
  
  // Set pin inputs/outputs
  pinMode(movementSensor, INPUT);
  pinMode(buttonSensor, INPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  
  // Add interrupts for change in sensor values.
  attachInterrupt(digitalPinToInterrupt(movementSensor), movementLedToggle, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonSensor), buttonLedToggle, CHANGE); 
}

void loop()
{
  
}

void movementLedToggle() {
  // Read and print the motion sensor value and update the LED.
  int motionValue = digitalRead(movementSensor);
  
  Serial.println("Movement Interrupt");
  Serial.println(motionValue);
  
  digitalWrite(greenLed, motionValue);
}

void buttonLedToggle() {
  // Read and print the button sensor value and update the LED.
  int buttonValue = digitalRead(buttonSensor);
  
  Serial.println("Button Interrupt");
  Serial.println(buttonValue);
  
  digitalWrite(redLed, buttonValue);
}