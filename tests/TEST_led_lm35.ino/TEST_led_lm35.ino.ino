#define ADC_VREF_mV 3300.0
#define ADC_RESOLUTION 4096.0
#define PIN_LM35 32
#define LED_PIN_12 12
#define LED_PIN_14 14

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN_12, OUTPUT);
  pinMode(LED_PIN_14, OUTPUT);
}

void loop() {
  int adcVal = analogRead(PIN_LM35);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempC = milliVolt / 5;///milliVolt / 10;
  
  // Check if the temperature in Celsius is 0 (indicating a possible issue)
  if (tempC == 0) {
    Serial.println("Reading is invalid (check connection)");
  } else {
    float tempF = tempC * 9 / 5 + 32;
    
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println("°F");
  }

  // Blink LEDs in 1-second cycles
  digitalWrite(LED_PIN_12, HIGH);  // Turn LED on pin 12 ON
  digitalWrite(LED_PIN_14, LOW);   // Turn LED on pin 14 OFF
  Serial.println("LED 12 ON, LED 14 OFF");
  delay(500);  // Wait for 500ms
  
  digitalWrite(LED_PIN_12, LOW);   // Turn LED on pin 12 OFF
  digitalWrite(LED_PIN_14, HIGH);  // Turn LED on pin 14 ON
  Serial.println("LED 12 OFF, LED 14 ON");
  delay(500);  // Wait for 500ms
}
