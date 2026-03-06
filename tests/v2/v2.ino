/**********************************************************************************
 * TITLE: Firebase Web-UI + Manual Button + Sensor Hub (Relays, DHT, Ultrasonic)
 *
 * This code merges two projects:
 * 1. Firebase 4-relay control with manual buttons.
 * 2. Sensor reading (DHT11, Ultrasonic) and LED control.
 *
 * FEATURES:
 * - 4 Relays (Pins 2, 19, 18, 5) with 2-way Firebase sync and manual buttons.
 * - DHT11 Sensor (Pin 25) pushing Temp/Humidity to Firebase.
 * - Ultrasonic Sensor (Pins 27, 26) pushing distance (cm) to Firebase.
 * - LED 1 (Pin 12) controlled by /led variable in Firebase.
 * - LED 2 (Pin 14) ALSO controlled by /led variable in Firebase (USER MODIFICATION).
 **********************************************************************************/

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <AceButton.h>
#include <DHT.h>

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
#define RELAY3 18
#define RELAY4 5

// --- Button GPIOs ---
#define SwitchPin1 13
#define SwitchPin2 12
#define SwitchPin3 14
#define SwitchPin4 27 // Note: This conflicts with TRIG_PIN

// !!!!! --- CRITICAL PIN CONFLICT --- !!!!!
// Your sensor code uses Pin 27 for TRIG_PIN
// Your relay code uses Pin 27 for SwitchPin4
//
// I will change SwitchPin4 to a different pin, for example, GPIO 33.
// Please update this pin to an unused one on your board.
#undef SwitchPin4
#define SwitchPin4 33 // <--- CHANGED PIN. Update this to a free GPIO.
// !!!!! --------------------------------- !!!!!


// --- Sensor & LED GPIOs (from your template) ---
#define DHT_PIN 25
#define DHT_TYPE DHT11
#define TRIG_PIN 27 // This is now free
#define ECHO_PIN 26
#define LED_PIN_12 12 // Note: This conflicts with SwitchPin2
#define LED_PIN_14 14 // Note: This conflicts with SwitchPin3

// !!!!! --- CRITICAL PIN CONFLICTS --- !!!!!
// Your sensor code uses Pin 12 for LED_PIN_12
// Your relay code uses Pin 12 for SwitchPin2
//
// Your sensor code uses Pin 14 for LED_PIN_14
// Your relay code uses Pin 14 for SwitchPin3
//
// This is a major issue. You CANNOT use the same pin for a button and an LED.
//
// I will assume the SENSORS/LEDs are the new requirement and will
// move the BUTTONS to new pins.
//
// --- NEW PINS (Please update to match your circuit) ---
#undef SwitchPin2
#undef SwitchPin3
#undef LED_PIN_12 // Pin 12 is now an LED
#undef LED_PIN_14 // Pin 14 is now an LED

#define LED_PIN_12 12 
#define LED_PIN_14 14

#define SwitchPin2 32 // <--- MOVED PIN. Update to a free GPIO.
#define SwitchPin3 35 // <--- MOVED PIN. Update to a free GPIO.

// --- Ultrasonic Threshold ---
//#define ULTRASONIC_THRESHOLD 20.0 // 20 cm

// --- Firebase Setup ---
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- AceButtons ---
AceButton button1(SwitchPin1);
AceButton button2(SwitchPin2);
AceButton button3(SwitchPin3);
AceButton button4(SwitchPin4);

// --- DHT Sensor ---
DHT dht(DHT_PIN, DHT_TYPE);

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
      // digitalWrite(LED_PIN_14, newState ? HIGH : LOW); // <-- MODIFICATION: REMOVED this line
      Firebase.RTDB.setBool(&fbdo, "/relay1", newState);
      break;

    case SwitchPin2:
      currentState = (digitalRead(RELAY2) == LOW);
      newState = !currentState;
      digitalWrite(RELAY2, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay2", newState);
      break;

    case SwitchPin3:
      currentState = (digitalRead(RELAY3) == LOW);
      newState = !currentState;
      digitalWrite(RELAY3, newState ? LOW : HIGH);
      Firebase.RTDB.setBool(&fbdo, "/relay3", newState);
      break;

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
  float tempF = dht.readTemperature(); // Read as Fahrenheit (matches '68' in screenshot)

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
  
  // Push boolean to Firebase
  //Firebase.RTDB.setBool(&fbdo, "/ulsc", isClose);
  Firebase.RTDB.setInt(&fbdo, "/ulsc", (int)distanceCm);
  Serial.printf("Pushed to Firebase: ulsc Distance: %.2f cm", distanceCm);
}

void setup() {
  Serial.begin(115200);

  // --- Setup relay pins ---
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY1, HIGH); // OFF
  digitalWrite(RELAY2, HIGH); // OFF
  digitalWrite(RELAY3, HIGH); // OFF
  digitalWrite(RELAY4, HIGH); // OFF

  // --- Setup button pins ---
  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);

  // --- Setup Sensor & LED pins ---
  dht.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN_12, OUTPUT);
  pinMode(LED_PIN_14, OUTPUT);
  digitalWrite(LED_PIN_12, LOW); // OFF
  digitalWrite(LED_PIN_14, LOW); // OFF

  // --- Attach AceButtons ---
  ButtonConfig* config1 = button1.getButtonConfig();
  ButtonConfig* config2 = button2.getButtonConfig();
  ButtonConfig* config3 = button3.getButtonConfig();
  ButtonConfig* config4 = button4.getButtonConfig();
  config1->setEventHandler(handleEvent);
  config2->setEventHandler(handleEvent);
  config3->setEventHandler(handleEvent);
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
    bool r1, r2, r3, r4, led_state; // Renamed led12_state to led_state

    // --- Relay 1 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay1")) {
      r1 = fbdo.boolData();
      digitalWrite(RELAY1, r1 ? LOW : HIGH); // Active LOW relay
      digitalWrite(LED_PIN_14, r1 ? HIGH : LOW); // <-- MODIFICATION: ADDED this line
      // digitalWrite(LED_PIN_14, r1 ? HIGH : LOW); // <-- MODIFICATION: REMOVED this line
    }
    
    // --- Relay 2 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay2")) {
      r2 = fbdo.boolData();
      digitalWrite(RELAY2, r2 ? LOW : HIGH);
    }

    // --- Relay 3 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay3")) {
      r3 = fbdo.boolData();
      digitalWrite(RELAY3, r3 ? LOW : HIGH);
    }

    // --- Relay 4 ---
    if (Firebase.RTDB.getBool(&fbdo, "/relay4")) {
      r4 = fbdo.boolData();
      digitalWrite(RELAY4, r4 ? LOW : HIGH);
    }

    // --- LED 12 and LED 14 (from /led) ---
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
  button3.check();
  button4.check();
}