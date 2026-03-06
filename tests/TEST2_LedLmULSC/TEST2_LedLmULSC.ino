#define ADC_VREF_mV 3300.0
#define ADC_RESOLUTION 4096.0
#define PIN_LM35 32

#define LED_PIN_12 12    // Simple ON/OFF LED
#define LED_PIN_14 14    // Fading LED (PWM)

#define TRIG_PIN 27
#define ECHO_PIN 26

void setup() {
  Serial.begin(9600);

  pinMode(LED_PIN_12, OUTPUT);
  pinMode(LED_PIN_14, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("System Initialized...");
}

void loop() {
  // ----- LM35 Temperature Reading -----
  int adcVal = analogRead(PIN_LM35);
  float milliVolt = 2 * adcVal * (ADC_VREF_mV / ADC_RESOLUTION); ///mul by 2 as per calibration

  float tempC = milliVolt / 10.0;

  if (tempC == 0) {
    Serial.println("Reading is invalid (check connection)");
  } else {
    float tempF = tempC * 9.0 / 5.0 + 32.0;
    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.println(" °C");
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println(" °F");
  }

  // ----- Ultrasonic Distance Measurement -----
  long duration;
  float distanceCm;

  // Trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5m range)
  distanceCm = duration * 0.0343 / 2; // Convert time to distance (cm)

  if (duration == 0) {
    Serial.println("Distance reading invalid (check sensor wiring)");
  } else {
    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  // ----- LED Behavior -----
  // LED 12: simple ON/OFF
  static bool led12State = false;
  led12State = !led12State;
  digitalWrite(LED_PIN_12, led12State ? HIGH : LOW);

  if (led12State)
    Serial.println("LED 12 ON");
  else
    Serial.println("LED 12 OFF");

  // LED 14: brightness fade 0 → 255 → 0 in 5 seconds
  int fadeTime = 5000; // 5 seconds full fade cycle
  int stepDelay = fadeTime / (255 * 2); // Up and down steps total 510

  // Fade in
  for (int brightness = 0; brightness <= 255; brightness++) {
    analogWrite(LED_PIN_14, brightness);
    delay(stepDelay);
  }

  // Fade out
  for (int brightness = 255; brightness >= 0; brightness--) {
    analogWrite(LED_PIN_14, brightness);
    delay(stepDelay);
  }

  Serial.println("----------------------------");
}
