// Task 1.2P SIT315, by Ryan Waites

// Digital pin devices
int movementSensor = 2;
int led = 4;

void setup()
{
  Serial.begin(9600);
  
  // Set pin inputs/outputs
  pinMode(movementSensor, INPUT);
  pinMode(led, OUTPUT);
  
  // Add interrupt for change in sensor value.
  attachInterrupt(digitalPinToInterrupt(movementSensor), ledToggle, CHANGE);  
}

void loop()
{
  
}

void ledToggle() {
  // Read and print the sensor value and update the LED.
  int motionValue = digitalRead(movementSensor);
  Serial.println(motionValue);
  digitalWrite(led, motionValue);
}