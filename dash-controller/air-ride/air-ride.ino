#include <ArduinoJson.h>
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADS1X15.h>
#include <LittleFS.h>

/*
#define I2C_SLAVE_ADDRESS 0x42
#define SDA_PIN_HOST 14
#define SCL_PIN_HOST 15
*/

const float        x_MinPressure       = 0;
const float        x_MaxPressure       = 30;

const uint8_t      x_ModeTravel        = 0;
const uint8_t      x_ModeManual        = 1;
const uint8_t      x_ModeLevel         = 2;

const uint8_t      x_RightInflate      = 0;
const uint8_t      x_RightDeflate      = 1;
const uint8_t      x_LeftDeflate       = 2;
const uint8_t      x_LeftInflate       = 3;

const uint8_t      x_LeftADS           = 0;
const uint8_t      x_RightADS          = 1;
const uint8_t      x_TankADS           = 2;

const int LED_PIN = 25;
const size_t bufferSize = 1024;
char jsonRxBuffer[bufferSize];
uint16_t iRxBuffer = 0;

const uint8_t outputPins[] = {9, 8, 7, 6};

Adafruit_MPU6050 mpu;

Adafruit_ADS1115 ads;

uint8_t mode = x_ModeManual;

struct Vector3 {
  float x, y, z;
};

struct Config {
  Vector3 levelAccel;
  float   minPressure;
  float   maxPressure;
};

Config config;

Vector3 rawAverage;
Vector3 correctedAverage;

float pressure[3];

// Moving average buffer
const int avgWindowSize = 3;
sensors_vec_t smoothedBuffer[avgWindowSize];
int bufferIndex = 0;
bool bufferFilled = false;

float round1(float value) {
   return (int)(value * 10 + 0.5) / 10.0;
}

float round2(float value) {
   return (int)(value * 100 + 0.5) / 100.0;
}

void send(const char * msg) {
  Serial.println(msg);
  Serial2.println(msg);
}

// ----- Calibration Storage -----

bool saveConfig() {
  File file = LittleFS.open("/config.dat", "w");
  if (!file) return false;
  file.write((uint8_t*)&config, sizeof(config));
  file.close();
  return true;
}

bool loadConfig() {
  File file = LittleFS.open("/config.dat", "r");
//  if (!file || file.size() != sizeof(levelAccel)) return false;
  if (!file) return false;
  file.read((uint8_t*)&config, sizeof(config));
  file.close();
  return true;
}

Vector3 getCorrectionVector(Vector3 smoothedValue) {
  return {
    smoothedValue.x - config.levelAccel.x,
    smoothedValue.y - config.levelAccel.y,
    smoothedValue.z - config.levelAccel.z
  };
}

Vector3 getSmoothed(sensors_vec_t currentAccel) {
  smoothedBuffer[bufferIndex] = currentAccel;
  bufferIndex = (bufferIndex + 1) % avgWindowSize;

  if (bufferIndex == 0) bufferFilled = true;

  int count = bufferFilled ? avgWindowSize : bufferIndex;

  Vector3 sum = {0, 0, 0};
  for (int i = 0; i < count; i++) {
    sum.x += smoothedBuffer[i].x;
    sum.y += smoothedBuffer[i].y;
    sum.z += smoothedBuffer[i].z;
  }

  return {
    sum.x / count,
    sum.y / count,
    sum.z / count
  };
}

void stop() {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(outputPins[i], HIGH);
  }
}

void clamp() {
  for (uint8_t i = 0; i < 2; i++) {
    if (pressure[i] >= config.maxPressure) {
      digitalWrite(outputPins[i == 0 ? x_LeftInflate : x_RightInflate], HIGH);
    }
    if (pressure[i] <= config.minPressure) {
      digitalWrite(outputPins[i == 0 ? x_LeftDeflate : x_RightDeflate], HIGH);
    }
  }
}

void printTiltDirection(Vector3 correction) {
  const float threshold = 0.2;

  if (correction.y > threshold)
    Serial.println("Tilted BACKWARD");
  else if (correction.y < -threshold)
    Serial.println("Tilted FORWARD");

  if (correction.x > threshold)
    Serial.println("Tilted RIGHT");
  else if (correction.x < -threshold)
    Serial.println("Tilted LEFT");
}

void setMode(const JsonVariant & m) {
  if (m == "travel") {
    mode = x_ModeTravel;
  } else if (m == "manual") {
    mode = x_ModeManual;
  } else if (m == "level") {
    mode = x_ModeLevel;
  } else {
    send("{\"error\":\"unexpected mode\"}");
    return;
  }
}

void setConfig(const JsonVariant & json) {
  if (json["minPressure"]) {
    config.minPressure = json["minPressure"];
    if (config.minPressure < x_MinPressure) {
      config.minPressure = x_MinPressure;
    }
  }
  if (json["maxPressure"]) {
    config.maxPressure = json["maxPressure"];
    if (config.maxPressure > x_MaxPressure) {
      config.maxPressure = x_MaxPressure;
    }
  }

  saveConfig();
}

void sendStatus() {
  JsonDocument doc;

  doc["mode"] = (mode == x_ModeTravel ? "travel" : (mode == x_ModeManual ? "manual" : "level"));

  doc["config"]["minPressure"] = config.minPressure;
  doc["config"]["maxPressure"] = config.maxPressure;

  doc["accel"]["x"] = round2(correctedAverage.x);
  doc["accel"]["y"] = round2(correctedAverage.y);
  doc["accel"]["z"] = round2(correctedAverage.z);

  doc["pressure"]["left"] = round(pressure[x_LeftADS]);
  doc["pressure"]["right"] = round(pressure[x_RightADS]);
  doc["pressure"]["tank"] = round(pressure[x_TankADS]);

  serializeJson(doc, Serial);
  Serial.println();
  serializeJson(doc, Serial2);
  Serial2.println();
}

void setLevel() {
  config.levelAccel = rawAverage;

  if (saveConfig()) {
    send("{\"info\":\"Saved calibration\"}");
  }
  else {
    send("{\"info\":\"Save calibration failed\"}");
  }
}


void handleManual(const JsonVariant & json) {
  bool left;
  bool up;
  uint8_t pressureIndex;

  mode = x_ModeManual;

  if (json == "stop") {
    stop();
  } else {
    left = json["left"];
    if (!left && !json["right"]) {
      send("{\"error\":\"unexpected side\"}");
      return;
    }
      
    up = json[left ? "left" : "right"] == "up";

    if (!up && (json[left ? "left" : "right"] != "down")) {
      send("{\"error\":\"unexpected direction\"}");
      return;
    }

    pressureIndex = (left ? x_LeftADS : x_RightADS);
    if (up && pressure[pressureIndex] >= config.maxPressure) {
      send("{\"warn\":\"max pressure reached\"}");
      return;
    }

    if (!up && pressure[pressureIndex] <= config.minPressure) {
      send("{\"warn\":\"min pressure reached\"}");
      return;
    }

    digitalWrite(outputPins[left ? (up ? x_LeftInflate : x_LeftDeflate) : (up ? x_RightInflate : x_RightDeflate)], LOW);
  }
}

void parseJsonRxBuffer() {
  bool parse = false;

  while (Serial2.available() > 0) {
    char c = Serial2.read();
    if (c == '\n' || c == '\r' || c == '\0') {
      if (iRxBuffer == 0) {
        continue;
      }
      jsonRxBuffer[iRxBuffer] = '\0';
      parse = true;
      break;
    }
    if (iRxBuffer >= bufferSize) {
      iRxBuffer = 0;
    }
    jsonRxBuffer[iRxBuffer++] = c;
  }

  if (!parse) {
    return;
  }

  JsonDocument doc;

  // Ignore non-JSON payloads
  if (iRxBuffer == 0 || jsonRxBuffer[0] != '{') {
    iRxBuffer = 0;
    return;
  }

  Serial.println(jsonRxBuffer);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonRxBuffer);

  iRxBuffer = 0;

  // Test if parsing succeeds.
  if (error) {
    Serial.printf("{\"error\":\"%s\"}\n", error.f_str());
    Serial2.printf("{\"error\":\"%s\"}\n", error.f_str());
    return;
  }

  if (doc["ping"]) {
    send("{\"ping\":\"ack\"}");
    return;
  }
  
  if (doc["config"]) {
    setConfig(doc["config"]);
    return;
  }

  if (doc["mode"]) {
    setMode(doc["mode"]);
    return;
  }

  if (doc["req"] == "status") {
    sendStatus();
    return;
  }

  if (doc["set"] == "level") {
    setLevel();
    return;
  }

  if (doc["manual"]) {
    handleManual(doc["manual"]);
  }
}

void readAccel() {
  static unsigned long lastReadTimestamp = 0;

  if (millis() - lastReadTimestamp > 20) {
    lastReadTimestamp = millis();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    a.acceleration.x *= -1; // flip x axis
    rawAverage = getSmoothed(a.acceleration);
    correctedAverage = getCorrectionVector(rawAverage);
    // printTiltDirection(correctedAverage);
  }
}

void readADS() {
  static unsigned long lastReadTimestamp = 0;

  if (millis() - lastReadTimestamp > 10) {
    lastReadTimestamp = millis();

    for (int i = 0; i < 3; i++) {
      int16_t rawADC = ads.readADC_SingleEnded(i);
      float voltage = rawADC * 4.096 / 32768.0;

      pressure[i] = (voltage - 0.5) * (i == 2 ? 37.5 : 7.5);
      if (pressure[i] < 0) pressure[i] = 0; // Clamp to 0 PSI
    }
  }
}

void blinkLED() {
  static unsigned long lastBlinkTimestamp = 0;
  static bool bBlink = false;

  if (millis() - lastBlinkTimestamp > 1000) {
    digitalWrite(LED_PIN, bBlink ? HIGH : LOW);
    bBlink = !bBlink;
    lastBlinkTimestamp = millis();
    // sendStatus();
    // send("{\"test\":\"this\"}");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial2.setTX(4);
  Serial2.setRX(5);
  Serial2.setCTS(UART_PIN_NOT_DEFINED);
  Serial2.setRTS(UART_PIN_NOT_DEFINED);
  Serial2.begin(38400);

  // Initialize I2C on Wire1 using GP14 (SDA) and GP15 (SCL)
  Wire1.setSDA(14);
  Wire1.setSCL(15);
  Wire1.begin();

  Serial.println("MPU6050 test on Wire1 (GP14/GP15)");

  // Initialize MPU6050 with Wire1
  if (!mpu.begin(0x68, &Wire1)) {
    Serial.println("❌ Failed to find MPU6050 chip on Wire1!");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  if (!loadConfig()) {
    send("{\"info\":\"No saved config, using defaults\"}");
    config.levelAccel = {0, 0, 9.8}; // fallback
    config.minPressure = 5.0;
    config.maxPressure = 25.0;
  } else {
    send("{\"info\":\"Configuration loaded from flash\"}");
  }

  // ---------- Setup ADS1115 on Wire (GP16/GP17) ----------
  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();

  if (!ads.begin(0x48, &Wire)) {
    Serial.println("❌ ADS1115 not found on Wire (GP16/GP17)");
    while (1);
  }
  ads.setGain(GAIN_ONE); // +/-4.096V range
  Serial.println("✅ ADS1115 initialized on Wire");

  for (int i = 0; i < sizeof(outputPins); i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], HIGH);
  }
}

void loop() {
  parseJsonRxBuffer();
  readAccel();
  readADS();
  clamp();

  blinkLED();
}
