#include <DHT.h>

// ----- DHT11 Setup -----
#define DHT_PIN 25
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ----- Ultrasonic Sensor Setup -----
#define TRIG_PIN 27
#define ECHO_PIN 26

// ----- LED Pins -----
#define LED_PIN_12 12    // Simple ON/OFF LED
#define LED_PIN_14 14    // Fading LED (PWM)

// ------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  dht.begin(); // Initialize DHT11 sensor

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN_12, OUTPUT);
  pinMode(LED_PIN_14, OUTPUT);

  Serial.println("System Initialized...");
}

// ------------------------------------------------------------------

void loop() {
  // ----- DHT11 Temperature & Humidity -----
  float humi = dht.readHumidity();
  float tempC = dht.readTemperature();
  float tempF = dht.readTemperature(true);

  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT11 sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print(" °C  ~  ");
    Serial.print(tempF);
    Serial.println(" °F");
  }

  // ----- Ultrasonic Distance Measurement -----
  long duration;
  float distanceCm;

  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse
  duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms (~5m range)
  distanceCm = duration * 0.0343 / 2; // Convert time to distance (cm)

  if (duration == 0) {
    Serial.println("Distance reading invalid (check sensor wiring)");
  } else {
    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.println(" cm");
  }

  // ----- LED Behavior -----
  // LED 12: simple ON/OFF toggle
  static bool led12State = false;
  led12State = !led12State;
  digitalWrite(LED_PIN_12, led12State ? HIGH : LOW);
  Serial.println(led12State ? "LED 12 ON" : "LED 12 OFF");

  // LED 14: fade in and out (brightness 0→255→0 in 5 seconds)
  int fadeTime = 5000; // total fade cycle time
  int stepDelay = fadeTime / (255 * 2); // time per brightness step

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
