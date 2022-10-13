// Task 1.4D SIT315, by Ryan Waites

// Digital pin devices
int movementSensor = 2;
int buttonSensor = 3;
int greenLed = 4;
int redLed = 5;
int whiteLed = 6;
int onboardLed = 13;

void setup()
{
  Serial.begin(9600);
  
  // Set pin inputs/outputs
  pinMode(movementSensor, INPUT);
  pinMode(buttonSensor, INPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(whiteLed, OUTPUT);
  pinMode(onboardLed, OUTPUT);
  
  // Add interrupts for change in sensor values.
  attachInterrupt(digitalPinToInterrupt(movementSensor), movementLedToggle, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonSensor), buttonLedToggle, CHANGE);
  
  // Turn on port B interrupts.
  PCICR |= 0b00000001;
  
  // Enable PCINT0 used for PIR #2 interrupt.
  PCMSK0 = 0b00000001;
  
  // Interrupt based on timer.
  startTimer();
  
}

void loop()
{
  
}

void movementLedToggle() {
  // Read and update the LED.
  int motionValue = digitalRead(movementSensor);
  
  Serial.println("PIR #1 Interrupt");
  
  digitalWrite(greenLed, motionValue);
}

void buttonLedToggle() {
  // Read and update the LED.
  int buttonValue = digitalRead(buttonSensor);
  
  Serial.println("Button Interrupt");
  digitalWrite(redLed, buttonValue);
}

void startTimer(){
  noInterrupts();
  
  // Calculate OCR1A value based on with 1024 prescaler. 
  // EX = mhz /(prescale * hz)-1 = 16000000 / (1024 * 0.5)-1
  uint16_t target = 16000000 / (1024 * 0.2) - 1;
  
  // Clear the registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  
  // Set timer compare
  OCR1A = target;
  
  // Set prescaler 1024
  TCCR1B |= (1 << CS12 | 1 << CS10);
  
  // Enable output compare match interrupt.
  TIMSK1 |= (1 << OCIE1A);
  
  // Enable CTC - Clear Timer on Compare
  TCCR1B |= (1 << WGM12);
  
  interrupts();
}

// Timer interupt function.
ISR(TIMER1_COMPA_vect){
  digitalWrite(onboardLed, !digitalRead(onboardLed));
  Serial.println("Timer Interrupt");
}

// Interupt function for PIR Sensor #2 on port B
ISR(PCINT0_vect) {
  digitalWrite(whiteLed, !digitalRead(whiteLed));
  Serial.println("PIR #2 Interrupt");
}