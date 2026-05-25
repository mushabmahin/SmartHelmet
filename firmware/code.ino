#include <HardwareSerial.h>
#include <Wire.h>
#include <math.h>
#include <TinyGPS++.h>
#include <DHT.h>

// ---------------- PIN DEFINITIONS ----------------
#define GSM_RX 25
#define GSM_TX 26
#define GPS_RX 16
#define GPS_TX 17
#define BUTTON_PIN 12
#define BUZZER_PIN 32
#define IR_PIN 13
#define MPU_ADDR 0x68
#define DHT_PIN 14
#define FAN_PIN 33//pin 33

// ---------------- GSM ----------------
HardwareSerial sim800(2);
//919995681865
const char* emergencyNumber = "+919995681865";

// ---------------- GPS ----------------
TinyGPSPlus gps;
HardwareSerial gpsSerial(1);

float lastValidLat = 0.0;
float lastValidLng = 0.0;
bool gpsFixAvailable = false;

// ---------------- HELMET STATUS ----------------
bool helmetWorn = false;

// ---------------- ACCIDENT CONTROL ----------------
bool countdownActive = false;
unsigned long countdownStart = 0;
const unsigned long cancelTime = 10000;

unsigned long lastBeepTime = 0;
bool buzzerState = false;

#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

float tempThreshold = 30.0;   // Temperature to turn fan ON

// ================= FUNCTION DECLARATIONS =================
void initializeSIM800();
void detectImpact();
void sendSMS(const char* number, const char* message);
void makeCall(const char* number);
void checkForIncomingSMS();
void readSMS(int index);
void updateGPS();
String getCurrentLocationLink();
void sendEmergencyAlert();
void sendCommand(String cmd);
void readResponse();
void checkHelmetStatus();

// ======================================================
// ===================== SETUP ==========================
// ======================================================

void setup() {

  Serial.begin(115200);
 pinMode(2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);

  digitalWrite(BUZZER_PIN, LOW);

  // GSM INIT
  sim800.begin(9600, SERIAL_8N1, GSM_RX, GSM_TX);
  delay(3000);
  initializeSIM800();

  // GPS INIT
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // MPU INIT
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("System Ready ✅");
  pinMode(FAN_PIN, OUTPUT);
digitalWrite(FAN_PIN, LOW);

dht.begin();
  digitalWrite(2, HIGH);
}

void loop() {

  updateGPS();
  checkHelmetStatus();
  checkForIncomingSMS();
  controlHelmetFan();

  // Accident detection ONLY if helmet worn
  if (helmetWorn) {
    detectImpact();
  }
  

  if (countdownActive) {

    if (millis() - lastBeepTime >= 500) {
      lastBeepTime = millis();
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
    }

    if (digitalRead(BUTTON_PIN) == LOW) {
      countdownActive = false;
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("Emergency Cancelled");
      delay(500);
    }

    if (millis() - countdownStart >= cancelTime) {
      digitalWrite(BUZZER_PIN, LOW);
      sendEmergencyAlert();
      countdownActive = false;
    }
  }
  //delay(30);
}

// ======================================================
// ================= HELMET DETECTION ===================
// ======================================================

void checkHelmetStatus() {

  int irValue = digitalRead(IR_PIN);

  if (irValue == LOW) {
    helmetWorn = true;
    Serial.println("Helmet ON");
  } 
  else {
    helmetWorn = false;
    Serial.println("Helmet OFF");
  }
}

// ======================================================
// ================= IMPACT DETECTION ===================
// ======================================================

void detectImpact() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  float totalAcc = sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g);
    Serial.print("TOTAL G-FORCE: ");
  Serial.print(totalAcc,3);
  Serial.println(" g");

  if (totalAcc >= 3 && !countdownActive) {
    Serial.println("Impact Detected!");
    countdownActive = true;
    countdownStart = millis();
    lastBeepTime = millis();
  }
}

// ======================================================
// ================= GPS FUNCTIONS ======================
// ======================================================

void updateGPS() {

  while (gpsSerial.available()) {

    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated() && gps.location.isValid()) {

      lastValidLat = gps.location.lat();
      lastValidLng = gps.location.lng();
      gpsFixAvailable = true;

      Serial.print("Stored GPS: ");
      Serial.print(lastValidLat, 6);
      Serial.print(" , ");
      Serial.println(lastValidLng, 6);
    }
  }
}

String getCurrentLocationLink() {

  if (gpsFixAvailable) {

    String link = "https://www.google.com/maps?q=" +
                  String(lastValidLat, 6) + "," +
                  String(lastValidLng, 6);

    return link;

  } else {

    return "https://maps.app.goo.gl/k83NGW5sJEDzMnkb7?g_st=aw";
  }
}

// ======================================================
// ================= EMERGENCY ALERT ====================
// ======================================================

void sendEmergencyAlert() {

  Serial.println("Sending Emergency Alert...");

  String message = "🚨 ACCIDENT ALERT!\n";
  message += "Helmet worn and accident detected.\n";
  message += getCurrentLocationLink();

  sendSMS(emergencyNumber, message.c_str());
  makeCall(emergencyNumber);
}

// ======================================================
// ================= SMS COMMAND ========================
// ======================================================

void checkForIncomingSMS() {

  if (sim800.available()) {

    String response = sim800.readString();

    if (response.indexOf("+CMTI:") != -1) {

      int index = response.substring(response.lastIndexOf(",") + 1).toInt();
      readSMS(index);
    }
  }
}

void readSMS(int index) {

  sim800.print("AT+CMGR=");
  sim800.println(index);
  delay(2000);

  String smsData = "";
  while (sim800.available()) {
    smsData += (char)sim800.read();
  }

  if (smsData.indexOf("GET LOCATION") != -1) {

    Serial.println("Location request received");

    String locationLink = getCurrentLocationLink();
    sendSMS(emergencyNumber, locationLink.c_str());
  }

  sim800.print("AT+CMGD=");
  sim800.println(index);
  delay(1000);
}

// ======================================================
// ================= GSM BASIC ==========================
// ======================================================

void initializeSIM800() {
  sendCommand("AT");
  sendCommand("ATE0");
  sendCommand("AT+CMGF=1");
  sendCommand("AT+CNMI=2,1,0,0,0");
}

void sendSMS(const char* number, const char* message) {

  sim800.print("AT+CMGS=\"");
  sim800.print(number);
  sim800.println("\"");
  delay(1000);

  sim800.print(message);
  delay(500);
  sim800.write(26);
  delay(5000);
}

void makeCall(const char* number) {

  sim800.print("ATD");
  sim800.print(number);
  sim800.println(";");

  delay(20000);
  sim800.println("ATH");
}

void sendCommand(String cmd) {
  sim800.println(cmd);
  delay(1000);
  readResponse();
}

void readResponse() {
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

void controlHelmetFan() {

  // If helmet is NOT worn → fan OFF immediately
  if (!helmetWorn) {
    digitalWrite(FAN_PIN, LOW);
    Serial.println("Helmet OFF → Fan Disabled");
    return;
  }

  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("DHT read failed");
    return;
  }

  Serial.print("Helmet Temp: ");
  Serial.println(temperature);

  if (temperature > tempThreshold) {
    digitalWrite(FAN_PIN, HIGH);
    Serial.println("Fan ON");
  } 
  else {
    digitalWrite(FAN_PIN, LOW);
    Serial.println("Fan OFF");
  }
}
