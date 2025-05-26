#include <ArduinoJson.h>
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LittleFS.h>

/*
#define I2C_SLAVE_ADDRESS 0x42
#define SDA_PIN_HOST 14
#define SCL_PIN_HOST 15
*/

const uint8_t      x_ModeTravel        = 0;
const uint8_t      x_ModeManual        = 1;
const uint8_t      x_ModeLevel         = 2;

const uint8_t      x_RightInflate      = 0;
const uint8_t      x_RightDeflate      = 1;
const uint8_t      x_LeftDeflate       = 2;
const uint8_t      x_LeftInflate       = 3;

const int LED_PIN = 25;
const size_t bufferSize = 1024;
char jsonRxBuffer[bufferSize];
uint16_t iRxBuffer = 0;

const uint8_t outputPins[] = {9, 8, 7, 6};

Adafruit_MPU6050 mpu;

uint8_t mode = x_ModeManual;

struct Vector3 {
  float x, y, z;
};

Vector3 levelAccel;
Vector3 rawAverage;
Vector3 correctedAverage;

// Moving average buffer
const int avgWindowSize = 10;
sensors_vec_t smoothedBuffer[avgWindowSize];
int bufferIndex = 0;
bool bufferFilled = false;

// ----- Calibration Storage -----

bool saveCalibration() {
  levelAccel = rawAverage;
  File file = LittleFS.open("/cal.dat", "w");
  if (!file) return false;
  file.write((uint8_t*)&levelAccel, sizeof(levelAccel));
  file.close();
  return true;
}

bool loadCalibration() {
  File file = LittleFS.open("/cal.dat", "r");
//  if (!file || file.size() != sizeof(levelAccel)) return false;
  if (!file) return false;
  file.read((uint8_t*)&levelAccel, sizeof(levelAccel));
  file.close();
  return true;
}

Vector3 getCorrectionVector(Vector3 smoothedValue) {
  return {
    smoothedValue.x - levelAccel.x,
    smoothedValue.y - levelAccel.y,
    smoothedValue.z - levelAccel.z
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

void printTiltDirection(Vector3 correction) {
  const float threshold = 0.2;

  if (correction.y > threshold)
    Serial2.println("Tilted BACKWARD");
  else if (correction.y < -threshold)
    Serial2.println("Tilted FORWARD");

  if (correction.x > threshold)
    Serial2.println("Tilted RIGHT");
  else if (correction.x < -threshold)
    Serial2.println("Tilted LEFT");
}

void sendStatus() {
  Serial.println("sendStatus");
}

void setLevel() {
  if (saveCalibration()) {
    Serial2.println("Saved calibration");
  }
  else {
    Serial2.println("Save calibration failed");
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

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonRxBuffer);

  iRxBuffer = 0;

  // Test if parsing succeeds.
  if (error) {
    Serial2.printf("{\"error\":\"%s\"}\n", error.f_str());
    return;
  }

  if (doc["mode"]) {
    if (doc["mode"] == "travel") {
      mode = x_ModeTravel;
    } else if (doc["mode"] == "manual") {
      mode = x_ModeManual;
    } else if (doc["mode"] == "level") {
      mode = x_ModeLevel;
    } else {
      Serial2.println("{\"error\":\"unexpected mode\"}");
      return;
    }
    Serial2.println("{\"ack\":\"mode\"}");
    return;
  }

  if (doc["ping"]) {
    Serial2.println("{\"ping\":\"ack\"}");
    return;
  }
  
  if (doc["status"] == "req") {
    sendStatus();
    return;
  }

  if (doc["set"] == "level") {
    if (saveCalibration()) {
      Serial2.println("{\"set\":\"ack\"}");
    } else {
      Serial2.println("{\"set\":\"nack\"}");
    }
    return;
  }

  if (doc["manual"]) {
    mode = x_ModeManual;
  
    if (doc["manual"] == "stop") {
      for (uint8_t i = 0; i < 4; i++) {
        digitalWrite(outputPins[i], HIGH);
      }
    } else if (doc["manual"]["left"]) {
      if (doc["manual"]["left"] == "up" /* && PSI(x_LeftBag) < x_MaxPSI */) {
        digitalWrite(outputPins[x_LeftInflate], LOW);
      }
      else if (doc["manual"]["left"] == "down" /* && PSI(x_LeftBag) > x_MinPSI */) {
        digitalWrite(outputPins[x_LeftDeflate], LOW);
      }
      else {
        Serial2.println("{\"error\":\"unexpected command\"}");
        return;
      }
    } else if (doc["manual"]["right"]) {
      if (doc["manual"]["right"] == "up" /* && PSI(x_LeftBag) < x_MaxPSI */) {
        digitalWrite(outputPins[x_RightInflate], LOW);
      }
      else if (doc["manual"]["right"] == "down" /* && PSI(x_LeftBag) > x_MinPSI */) {
        digitalWrite(outputPins[x_RightDeflate], LOW);
      }
      else {
        Serial2.println("{\"error\":\"unexpected command\"}");
        return;
      }
    }
  
    Serial2.println("{\"manual\":\"ack\"}");
  }
}

void readAccel() {
  static unsigned long lastReadTimestamp = 0;

  if (millis() - lastReadTimestamp > 100) {
    lastReadTimestamp = millis();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    rawAverage = getSmoothed(a.acceleration);
    correctedAverage = getCorrectionVector(rawAverage);
    printTiltDirection(correctedAverage);
  }
}

void blinkLED() {
  static unsigned long lastBlinkTimestamp = 0;
  static bool bBlink = false;

  if (millis() - lastBlinkTimestamp > 1000) {
    digitalWrite(LED_PIN, bBlink ? HIGH : LOW);
    bBlink = !bBlink;
    lastBlinkTimestamp = millis();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial2.setTX(4);
  Serial2.setRX(5);
  Serial2.begin(9600);

  // Initialize I2C on Wire1 using GP14 (SDA) and GP15 (SCL)
  Wire1.setSDA(14);
  Wire1.setSCL(15);
  Wire1.begin();

  Serial.println("MPU6050 test on Wire1 (GP14/GP15)");

  // Initialize MPU6050 with Wire1
  if (!mpu.begin(0x68, &Wire1)) {
    Serial.println("Failed to find MPU6050 chip on Wire1!");
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  if (!loadCalibration()) {
    Serial2.println("No saved calibration, using default (0, 0, 9.8)");
    levelAccel = {0, 0, 9.8}; // fallback
  } else {
    Serial2.println("Calibration loaded from flash.");
  }

/*
  Wire.setSDA(SDA_PIN_HOST);
  Wire.setSCL(SCL_PIN_HOST);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Wire1.setSDA(SDA_PIN_SERVO);
  Wire1.setSCL(SCL_PIN_SERVO);
  Wire1.begin();
*/

  for (int i = 0; i < sizeof(outputPins); i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], HIGH);
  }
}

void loop() {
  parseJsonRxBuffer();
  delay(10);
  readAccel();
  delay(10);

  blinkLED();
}
