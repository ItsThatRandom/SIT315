// Task 1.1P SIT315, by Ryan Waites

// Digital pin devices
int movementSensor = 2;
int led = 4;

void setup()
{
  Serial.begin(9600);
  
  // Set pin inputs/outputs
  pinMode(movementSensor, INPUT);
  pinMode(led, OUTPUT);
}

void loop()
{
  // Read motion sensor input, print value and update led output.
  int motionValue = digitalRead(movementSensor);
  Serial.println(motionValue);
  digitalWrite(led, motionValue);
}