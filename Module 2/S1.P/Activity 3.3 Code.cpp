// C++ code
//

const byte LED_PIN = 13;
const byte METER_PIN = A4;


void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);
      
  Serial.begin(9600);
  
  startTimer(0.5);
}

void loop()
{
  // Read raw value (int 0 -> 1024)
  int meterValue = analogRead(METER_PIN);
  
  // Convert to Double (0.00 -> 4.00)
  double value = (meterValue/256.0);
  
  // Print value for testing
  Serial.println(value);
  
  // Pass new value and delay for 1 second
  startTimer(value); 
  delay(1000);
}



void startTimer(double timerFrequency){
  noInterrupts();
  
    
  // Calculate OCR1A value based on 'timerFrequency' with
  // 1024 prescaler. 
  // EX = mhz /(prescale * hz)-1 = 16000000 / (1024 * 0.5)-1
  uint16_t target = 16000000 / (1024 * timerFrequency) - 1;;
  
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


ISR(TIMER1_COMPA_vect){
   digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);
}

