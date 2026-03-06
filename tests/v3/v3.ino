/**********************************************************************************
 * TITLE: Firebase Web-UI + Sensor Hub + Servo
 *
 * This code merges three projects:
 * 1. Firebase 4-relay control with manual buttons.
 * 2. Sensor reading (DHT11, Ultrasonic) and LED control.
 * 3. Servo motor control via Firebase.
 *
 * FEATURES:
 * - 3 Relays (Pins 2, 19, 5) with 2-way Firebase sync and manual buttons.
 * - 1 Servo Motor (Pin 13) controlled by /relay3 path in Firebase.
 * - DHT11 Sensor (Pin 25) pushing Temp/Humidity to Firebase.
 * - Ultrasonic Sensor (Pins 27, 26) pushing distance (cm) to Firebase.
 * - LED 1 (Pin 12) controlled by /led variable in Firebase.
 * - LED 2 (Pin 14) controlled by /relay1 variable in Firebase.
 **********************************************************************************/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <AceButton.h>
#include <DHT.h>
#include <ESP32Servo.h> // --- SERVO MERGE: Added Servo library ---

using namespace ace_button;

// --- Wi-Fi credentials ---
const char* ssid = "ysr_network";     // WiFi Name
const char* password = "1234567890";  // WiFi Password

// --- Firebase credentials ---
#define API_KEY "AIzaSyBwhAl9D7crGS_0vS1qhh4INrMEvXp0Wlk"
#define DATABASE_URL "https://tarp-aura-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "abc@gmail.com"
#define USER_PASSWORD "123456"

// --- Relay GPIOs (active LOW) ---
#define RELAY1 2
#define RELAY2 19
// #define RELAY3 18 // --- SERVO MERGE: This pin is now unused. Logic is replaced by servo.
#define RELAY4 5

// --- Button GPIOs ---

// !!!!! --- SERVO MERGE: PIN CONFLICT RESOLVED --- !!!!!
// Your new servo code required Pin 13.
// Your old code used Pin 13 for SwitchPin1.
// I have MOVED SwitchPin1 to GPIO 34 (a free pin).
// Pin 13 is now free for the servo.
//
#define SwitchPin1 34 // <--- MOVED PIN from 13.
// #define SwitchPin1 13 // Old pin
// !!!!! -------------------------------------------- !!!!!

#define SwitchPin2 32
// #define SwitchPin3 35 // --- SERVO MERGE: This button is now unused, as it controlled /relay3.
#define SwitchPin4 33

// --- Sensor & LED GPIOs ---
#define DHT_PIN 25
#define DHT_TYPE DHT11
#define TRIG_PIN 27
#define ECHO_PIN 26
#define LED_PIN_12 12
#define LED_PIN_14 14

// --- SERVO MERGE: Servo Pin Definition ---
static const int servoPin = 13; // As requested, using Pin 13

// --- Firebase Setup ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- AceButtons ---
AceButton button1(SwitchPin1);
AceButton button2(SwitchPin2);
// AceButton button3(SwitchPin3); // --- SERVO MERGE: Button 3 is no longer used.
AceButton button4(SwitchPin4);

// --- DHT Sensor ---
DHT dht(DHT_PIN, DHT_TYPE);

// --- SERVO MERGE: Servo Object ---
Servo servo1;
int currentServoPos = 90; // Holds the last known servo position

// --- Timer for non-blocking sensor reads ---
unsigned long lastSensorRead = 0;
const long sensorReadInterval = 5000; // 5 seconds

// --- Button Event Handler ---
void handleEvent(AceButton* button, uint8_t eventType, uint8_t /* buttonState */) {
  if (eventType != AceButton::kEventReleased) return;

  int id = button->getPin();
  bool currentState;
  bool newState;

  switch (id) {
    case SwitchPin1:
      currentState = (digitalRead(RELAY1) == LOW); // true if ON
      newState = !currentState; // Invert the state
      digitalWrite(RELAY1, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay1", newState);
      break;

    case SwitchPin2:
      currentState = (digitalRead(RELAY2) == LOW);
      newState = !currentState;
      digitalWrite(RELAY2, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay2", newState);
      break;

    /* --- SERVO MERGE: Removed Switch 3 Case ---
       This button is no longer used, as /relay3 is now
       an integer path for the servo, not a boolean for a relay.
    case SwitchPin3:
      currentState = (digitalRead(RELAY3) == LOW);
      newState = !currentState;
      digitalWrite(RELAY3, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay3", newState);
      break;
    */

    case SwitchPin4:
      currentState = (digitalRead(RELAY4) == LOW);
      newState = !currentState;
      digitalWrite(RELAY4, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay4", newState);
      break;
  }
}

// --- Sensor Reading Function ---
void readAndPushSensors() {
  Serial.println("Reading sensors...");

  // ----- 1. DHT11 Temperature & Humidity -----
  float humi = dht.readHumidity();
  float tempF = dht.readTemperature(); // Read as Fahrenheit

  if (isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT11 sensor!");
  } else {
    // Push to Firebase
    Firebase.RTDB.setInt(&fbdo, "/humidity", (int)humi);
    Firebase.RTDB.setInt(&fbdo, "/temperature", (int)tempF);
    Serial.printf("Pushed to Firebase: Humi: %d, Temp: %d\n", (int)humi, (int)tempF);
  }

  // ----- 2. Ultrasonic Distance Measurement -----
  long duration;
  float distanceCm;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  distanceCm = duration * 0.0343 / 2;
  
  Firebase.RTDB.setInt(&fbdo, "/ulsc", (int)distanceCm);
  Serial.printf("Pushed to Firebase: ulsc Distance: %.2f cm\n", distanceCm);
}

void setup() {
  Serial.begin(115200);

  // --- Setup relay pins ---
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  // pinMode(RELAY3, OUTPUT); // --- SERVO MERGE: Pin 18 (RELAY3) is no longer used.
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, HIGH); // OFF
  digitalWrite(RELAY2, HIGH); // OFF
  // digitalWrite(RELAY3, HIGH); // --- SERVO MERGE: Pin 18 (RELAY3) is no longer used.
  digitalWrite(RELAY4, HIGH); // OFF

  // --- Setup button pins ---
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  // pinMode(SwitchPin3, INPUT_PULLUP); // --- SERVO MERGE: Pin 35 (SwitchPin3) is no longer used.
  pinMode(SwitchPin4, INPUT_PULLUP);

  // --- Setup Sensor & LED pins ---
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN_12, OUTPUT);
  pinMode(LED_PIN_14, OUTPUT);
  digitalWrite(LED_PIN_12, LOW); // OFF
  digitalWrite(LED_PIN_14, LOW); // OFF

  // --- SERVO MERGE: Attach Servo ---
  servo1.attach(servoPin);
  servo1.write(currentServoPos); // Set servo to default 90-degree position on boot

  // --- Attach AceButtons ---
  ButtonConfig* config1 = button1.getButtonConfig();
  ButtonConfig* config2 = button2.getButtonConfig();
  // ButtonConfig* config3 = button3.getButtonConfig(); // --- SERVO MERGE: Button 3 is no longer used.
  ButtonConfig* config4 = button4.getButtonConfig();
  config1->setEventHandler(handleEvent);
  config2->setEventHandler(handleEvent);
  // config3->setEventHandler(handleEvent); // --- SERVO MERGE: Button 3 is no longer used.
  config4->setEventHandler(handleEvent);

  // --- Connect to Wi-Fi ---
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");

  // --- Connect to Firebase ---
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // --- 1. Check Firebase for remote updates ---
  if (Firebase.ready()) {
    bool r1, r2, r4, led_state; // Renamed led12_state to led_state

    // --- Relay 1 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay1")) {
      r1 = fbdo.boolData();
      digitalWrite(RELAY1, r1 ? LOW : HIGH); // Active LOW relay
      digitalWrite(LED_PIN_14, r1 ? HIGH : LOW);
    }
    
    // --- Relay 2 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay2")) {
      r2 = fbdo.boolData();
      digitalWrite(RELAY2, r2 ? LOW : HIGH);
    }

    // --- SERVO (replaces Relay 3) ---
    // We now read /relay3 as an INTEGER (0-180) to control the servo
    if (Firebase.RTDB.getInt(&fbdo, "/relay3")) {
      int newPos = fbdo.intData();
      
      // Safety check to ensure value is within servo range (0-180)
      if (newPos >= 0 && newPos <= 180) {
        
        // Only write to the servo if the position has actually changed
        if (newPos != currentServoPos) {
          currentServoPos = newPos;
          servo1.write(currentServoPos);
          Serial.printf("Servo position updated from Firebase: %d\n", currentServoPos);
        }
      } else {
        Serial.printf("Received invalid servo position from Firebase: %d\n", newPos);
      }
    }
    /* --- SERVO MERGE: Old Relay 3 logic removed ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay3")) {
      r3 = fbdo.boolData();
      digitalWrite(RELAY3, r3 ? LOW : HIGH);
    }
    */

    // --- Relay 4 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay4")) {
      r4 = fbdo.boolData();
      digitalWrite(RELAY4, r4 ? LOW : HIGH);
    }

    // --- LED 12 (from /led) ---
    if (Firebase.RTDB.getBool(&fbdo, "/led")) {
      led_state = fbdo.boolData();
      digitalWrite(LED_PIN_12, led_state ? HIGH : LOW);
    }

    // --- 2. Push Sensor Data (on interval) ---
    if (millis() - lastSensorRead > sensorReadInterval) {
      lastSensorRead = millis();
      readAndPushSensors();
    }
  }

  // --- 3. Check for manual button presses ---
  button1.check();
  button2.check();
  // button3.check(); // --- SERVO MERGE: Button 3 is no longer used.
  button4.check();
  
  // --- SERVO MERGE: Remove servo sweep loops ---
  // The original servo sweep loops from your example are removed.
  // The servo now ONLY moves when it gets a new value from Firebase.
}