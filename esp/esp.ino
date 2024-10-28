#include <Wire.h>
#include <DFRobot_MAX30102.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

DFRobot_MAX30102 particleSensor;
const int BEATLED = 15;
FirebaseData firebaseData;

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "D@edge30";
const char* password = "crce@306";
#define FIREBASE_HOST "https://pulse-wave-link-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "XyqDuPqdarzWsGbAR6zTyim2AIFG6FuNpf80dnqV"

int32_t SPO2;
int8_t SPO2Valid;
int32_t heartRate;
int8_t heartRateValid;
const int thresholdHeartRate = 170;
const int thresholdHeartRate1 = 40;
void setup() {
  Serial.begin(115200);
  delay(1000); // Allow time for serial monitor to open

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Initialize MAX30102 sensor
  while (!particleSensor.begin()) {
    Serial.println("MAX30102 was not found");
    delay(1000);
  }
  particleSensor.sensorConfiguration(50, SAMPLEAVG_4, MODE_MULTILED, SAMPLERATE_100, PULSEWIDTH_411, ADCRANGE_16384);
  pinMode(BEATLED, OUTPUT);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  float temperatureC = particleSensor.readTemperatureC();
  float temperatureF = (temperatureC * 9 / 5) + 32;
  int temperatureInt = (int)temperatureF;

  particleSensor.heartrateAndOxygenSaturation(&SPO2, &SPO2Valid, &heartRate, &heartRateValid);

  // Check if the SPO2 and heart rate readings are valid
  if (SPO2Valid && heartRateValid) {
    if (SPO2 < 70) SPO2 = 70;
    if (SPO2 > 100) SPO2 = 100;
    if (heartRate < 70) heartRate = 70;
    if (heartRate > thresholdHeartRate || heartRate < thresholdHeartRate1) {
      Serial.println("Heart rate exceeds safe threshold!");
      displayAlert();
    }

    // Display readings on OLED
    displayDataOnOLED(heartRate, SPO2, temperatureC);

    // Send data to Firebase
    sendDataToFirebase(heartRate, SPO2, temperatureC);
  } else {
    Serial.println("Invalid SPO2 and heart rate readings");
  }

  delay(10000); // Update every 10 seconds
}

void displayDataOnOLED(int heartRate, int SPO2, int temperature) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Heart Rate: ");
  display.print(heartRate);
  display.println(" BPM");
  display.print("SpO2: ");
  display.print(SPO2);
  display.println(" %");
  display.print("Temp: ");
  display.print(temperature);
  display.println(" C");
  display.display();
}

void sendDataToFirebase(int heartRate, int SPO2, int temperature) {
  if (Firebase.setInt(firebaseData, "/health/bpm", heartRate)) {
    Serial.println("Heart rate data sent to Firebase");
  } else {
    Serial.println("Failed to send heart rate data to Firebase");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/health/SpO2", SPO2)) {
    Serial.println("SPO2 data sent to Firebase");
  } else {
    Serial.println("Failed to send SPO2 data to Firebase");
    Serial.println(firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/health/tem", temperature)) {
    Serial.println("Temperature data sent to Firebase");
  } else {
    Serial.println("Failed to send temperature data to Firebase");
    Serial.println(firebaseData.errorReason());
  }
}

void displayAlert() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5);
  display.println("ALERT!");
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println("Heart Rate too high!");
  display.display();
  delay(2000); // Display the alert for 2 seconds
}