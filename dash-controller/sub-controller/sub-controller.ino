#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <LittleFS.h>

#define I2C_SLAVE_ADDRESS 0x42
#define SDA_PIN_HOST 12
#define SCL_PIN_HOST 13

// Create a PCA9685 instance on the default I2C address 0x40.
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire1);

#define SDA_PIN_SERVO 2
#define SCL_PIN_SERVO 3
#define SERVOMIN  400 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  2000 // this is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60 // Analog servos run at ~60 Hz updates
#define SERVO_RANGE 1000
#define SERVOS_IN_USE_MASK 0xF000
#define SERVO_COUNT 16
#define SERVO_CHANGE_RATE 2
#define STATUS_SAVE_DELAY 5000

const int LED_PIN = 25;
const size_t bufferSize = 1024;
char jsonRxBuffer[bufferSize];
uint16_t iRxBuffer = 0;
char jsonTxBuffer[bufferSize];
uint16_t iTxBuffer = 0;

const uint8_t inputPins[] = {22, 21, 20, 19, 26, 18, 17, 16};
// uint8_t pinMapCurrent = 0;

const uint8_t outputPins[] = {7, 6, 11, 10};

void txMessage(const char * msg) {
  noInterrupts();
  int len = sprintf(jsonTxBuffer, "%s", msg);
  iTxBuffer = (len > 0 ? len : 0);
  interrupts();
}

void txJson(const JsonVariant& json) {
  noInterrupts();
  iTxBuffer = serializeJson(json, jsonTxBuffer, bufferSize);
  interrupts();
}

class Gps {
public:
  void init() {
    Serial2.setTX(8);
    Serial2.setRX(9);
    Serial2.begin(9600);
  };

  void process() {
    if (isReady) {
      return;
    }
    while (Serial2.available() > 0) {
      char c = Serial2.read();
      gps.encode(c);
      if (gps.location.isUpdated()) {
        isReady = true;
        // send();
        return;
      }
    }
  };

  bool ready() {
    return isReady;
  };

  void send() {
    if (isReady){
      JsonDocument gpsDoc;
      gpsDoc["gps"]["location"]["lat"] = gps.location.lat();
      gpsDoc["gps"]["location"]["long"] = gps.location.lng();
      gpsDoc["gps"]["timestamp"]["year"] = gps.date.year();
      gpsDoc["gps"]["timestamp"]["month"] = gps.date.month();
      gpsDoc["gps"]["timestamp"]["day"] = gps.date.day();
      gpsDoc["gps"]["timestamp"]["hour"] = gps.time.hour();
      gpsDoc["gps"]["timestamp"]["minute"] = gps.time.minute();
      gpsDoc["gps"]["timestamp"]["second"] = gps.time.second();
      gpsDoc["gps"]["course"]["deg"] = gps.course.deg();
      gpsDoc["gps"]["altitude"]["feet"] = gps.altitude.feet();
      gpsDoc["gps"]["altitude"]["meters"] = gps.altitude.meters();
      gpsDoc["gps"]["speed"]["mph"] = (int)gps.speed.mph();
      gpsDoc["gps"]["speed"]["kmph"] = (int)gps.speed.kmph();
      gpsDoc["gps"]["satellites"] = gps.satellites.value();
      gpsDoc["gps"]["hdop"] = gps.hdop.hdop();

      txJson(gpsDoc);
      isReady = false;
    }
  };

private:
  // The TinyGPS++ object
  TinyGPSPlus gps;
  bool isReady = false;
};


class AirRide {
public:
  void init() {
    Serial1.setTX(0);
    Serial1.setRX(1);
    Serial1.begin(38400);
  };

  void process() {
//    if (isReady) {
  //    return;
    //}

    while (Serial1.available() > 0) {
      char c = Serial1.read();
      if (c == '\n' || c == '\r' || c == '\0') {
        if (iRxAirRideBuffer == 0) {
          continue;
        }
        rxAirRideBuffer[iRxAirRideBuffer] = '\0';
        isReady = true;
       // Serial.println();
        return;
      }
      if (iRxAirRideBuffer >= bufferSize) {
        iRxAirRideBuffer = 0;
      }
      rxAirRideBuffer[iRxAirRideBuffer++] = c;
  //    Serial.print(c);
    }

    if (millis() - lastStatusReq > 2000) {
      sendStatusReq();
    }
  };

  bool ready() {
    return isReady;
  };

  void send() {
    // txMessage(rxAirRideBuffer);
    noInterrupts();
    int len = sprintf(jsonTxBuffer, "{\"airRide\":%s}", rxAirRideBuffer);
    iTxBuffer = (len > 0 ? len : 0);
  //  Serial.println(jsonTxBuffer);
    interrupts();
    iRxAirRideBuffer = 0;
    isReady = false;
    sendStatusReq();
  };

private:
  char rxAirRideBuffer[bufferSize];
  uint16_t iRxAirRideBuffer = 0;
  bool isReady = false;
  unsigned long lastStatusReq = 0;

  void sendStatusReq() {
    Serial1.println("{\"req\":\"status\"}");
    lastStatusReq = millis();
  };
};

/*
bool readAirRide() {
  static unsigned long startedTimestamp = millis();
  static char rxAirRideBuffer[bufferSize];
  static uint16_t iRxAirRideBuffer = 0;

  while (Serial1.available() > 0) {
    char c = Serial1.read();
    if (c == '\n' || c == '\r' || c == '\0') {
      if (iRxAirRideBuffer == 0) {
        continue;
      }
      rxAirRideBuffer[iRxAirRideBuffer] = '\0';
      noInterrupts();
      int len = sprintf(jsonTxBuffer, "{\"airRide\":%s}", rxAirRideBuffer);
      iTxBuffer = (len > 0 ? len : 0);
      Serial.println(jsonTxBuffer);
      Serial.println(iTxBuffer);
      interrupts();
      iRxAirRideBuffer = 0;

      return true;
    }
    if (iRxAirRideBuffer >= bufferSize) {
      iRxAirRideBuffer = 0;

      return false;
    }
    rxAirRideBuffer[iRxAirRideBuffer++] = c;
  }

  return false;
}
*/

struct Servo {
  int pulse = 0;
  int targetPulse = 0;
};

Servo servosStatus[SERVO_COUNT];

int denormalizePulse(int pulse) {
  return map(pulse, 0, SERVO_RANGE, SERVOMIN, SERVOMAX);
}

int normalizePulse(int pulse) {
  return map(pulse, SERVOMIN, SERVOMAX, 0, SERVO_RANGE);
}

void handleServo(const JsonVariant& servo) {
  if (!servo["num"]) {
    txMessage("{\"error\":\"missing servo num value\"}");
    return;
  }
  uint8_t servoNum = servo["num"].as<uint8_t>();
  if (servoNum > 15) {
    txMessage("{\"error\":\"servo num out of range\"}");
    return;
  }
  if (!servo["pulse"]) {
    txMessage("{\"error\":\"missing servo pulse value\"}");
    return;
  }
  int pulse = servo["pulse"];
  if (pulse < 0 || pulse > SERVO_RANGE) {
    txMessage("{\"error\":\"servo pulse out of range\"}");
    return;
  }
  servosStatus[servoNum].targetPulse = denormalizePulse(pulse);
  // pwm.writeMicroseconds(servoNum, denormalizePulse(pulse));
}

void sendStatus() {
  JsonDocument doc;

  JsonArray servoArray = doc["servos"].to<JsonArray>();

  // Populate it from your C++ array
  for (size_t i = 0; i < SERVO_COUNT; ++i) {
    if (SERVOS_IN_USE_MASK & (1 << i)) {
      JsonObject servoObj = servoArray.add<JsonObject>();
      servoObj["num"] = i;
      servoObj["pulse"] = normalizePulse(servosStatus[i].pulse);
    }
  }

  txJson(doc);
}

void parseJsonRxBuffer() {
  JsonDocument doc;

  // Ignore non-JSON payloads
  if (iRxBuffer == 0 || jsonRxBuffer[0] != '{') {
    iRxBuffer = 0;
    return;
  }

  if (iTxBuffer != 0) {
    return;
  }

  Serial.println(jsonRxBuffer);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonRxBuffer);

  iRxBuffer = 0;

  // Test if parsing succeeds.
  if (error) {
    noInterrupts();
    int len = sprintf(jsonTxBuffer, "{\"error\":\"%s\"}", error.f_str());
    iTxBuffer = (len > 0 ? len : 0);
    interrupts();
    return;
  }

  if (doc["servo"]) {
    handleServo(doc["servo"]);
    txMessage("{\"servo\":\"ack\"}");

    return;
  }

  if (doc["servos"]) {
    JsonArray servos = doc["servos"];

    for (JsonVariant servo : servos) {
      handleServo(servo);
    }
    txMessage("{\"servos\":\"ack\"}");

    return;
  }

  if (doc["fan"]) {
    if (!doc["fan"]["speed"]) {
      txMessage("{\"error\":\"missing fan speed value\"}");
      return;
    }
    if (!doc["fan"]["state"]) {
      txMessage("{\"error\":\"missing fan state value\"}");
      return;
    }

    uint8_t fanSpeed = doc["fan"]["speed"].as<uint8_t>();
    if (fanSpeed > 3) {
      txMessage("{\"error\":\"fan speed of range\"}");
      return;
    }

    if (doc["fan"]["state"] != "on" && doc["fan"]["state"] != "off") {
      txMessage("{\"error\":\"invalid value for fan state\"}");
      return;
    }

    bool fanStateOn = (doc["fan"]["state"] == "on");

    for (uint8_t i = 0; i < 4; i++) {
      digitalWrite(outputPins[i], (fanStateOn && (i == fanSpeed)) ? LOW : HIGH);
    }

    txMessage("{\"fan\":\"ack\"}");

    return;
  }

  if (doc["relay"]) {
    if (doc["relay"] == "off") {
      for (uint8_t i = 0; i < sizeof(outputPins); i++) {
        digitalWrite(outputPins[i], HIGH);
      }
      txMessage("{\"relay\":\"ack\"}");
      return;      
    }
    if (!doc["relay"]["num"]) {
      txMessage("{\"error\":\"missing relay num value\"}");
      return;
    }
    uint8_t relayNum = doc["relay"]["num"].as<uint8_t>();
    if (relayNum >= sizeof(outputPins)) {
      txMessage("{\"error\":\"relay num out of range\"}");
      return;
    }
    if (!doc["relay"]["state"]) {
      txMessage("{\"error\":\"missing relay state value\"}");
      return;
    }
    if (doc["relay"]["state"] != "on" && doc["relay"]["state"] != "off") {
      txMessage("{\"error\":\"invalid value for relay state\"}");
      return;
    }

    digitalWrite(outputPins[relayNum], doc["relay"]["state"] == "on" ? LOW : HIGH);
    txMessage("{\"relay\":\"ack\"}");

    return;
  }

  if (doc["airRide"]) {
    serializeJson(doc["airRide"], Serial1);
    Serial1.println();

    return;
  }

  if (doc["req"]) {
    if (doc["req"] == "status") {
      sendStatus();
    }

    return;
  }

  if (doc["ping"]) {
    txMessage("{\"ping\":\"ack\"}");
    // force resending of current pinMap
    // pinMapCurrent = 0xFF;
  }
}

void receiveEvent(int howMany) {
  while (Wire.available() > 0) {
    char c = (char) Wire.read();
    if (c == '\n' || c == '\r' || c == '\0') {
      if (iRxBuffer == 0) {
        continue;
      }
      jsonRxBuffer[iRxBuffer] = '\0';
      break;
    }
    if (iRxBuffer >= bufferSize) {
      iRxBuffer = 0;
      break;
    }
    jsonRxBuffer[iRxBuffer++] = c;
  }
}

/*
void readHost() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r' || c == '\0') {
      if (iRxBuffer == 0) {
        continue;
      }
      jsonRxBuffer[iRxBuffer] = '\0';
      break;
    }
    if (iRxBuffer >= bufferSize) {
      iRxBuffer = 0;
      break;
    }
    jsonRxBuffer[iRxBuffer++] = c;
  }
}
*/

void requestEvent() {
  static int callCount = 0;
  callCount++;
  if (callCount > 16384) {
//    Serial.println();
    callCount = 0;
  }
  if (callCount % 256 == 0) {
//    Serial.print('.');
  }
  for (uint16_t i = 0; i < iTxBuffer; i++) {
    Wire.write(jsonTxBuffer[i]);
  }
  Wire.write('\0'); // Ensure null terminator is sent
  iTxBuffer = 0;
}


void readGPIO() {
  uint8_t pinMap = 0;

  for (int i = 0; i < sizeof(inputPins); i++) {
    pinMap |= (digitalRead(inputPins[i]) == HIGH ? 1 : 0) << i;
  }

  noInterrupts();
  int len = sprintf(jsonTxBuffer, "{\"12vInputs\":%d}", pinMap);
  iTxBuffer = (len > 0 ? len : 0);
  interrupts();
}

/*
bool readGPIO() {
  static unsigned long lastReportTimestamp = 0;
  uint8_t pinMapPrevious = pinMapCurrent;

  pinMapCurrent = 0;
  for (int i = 0; i < sizeof(inputPins); i++) {
    pinMapCurrent |= (digitalRead(inputPins[i]) == HIGH ? 1 : 0) << i;
  }

  if (pinMapCurrent != pinMapPrevious || (millis() - lastReportTimestamp > 500)) {
    noInterrupts();
    int len = sprintf(jsonTxBuffer, "{\"12vInputs\":%d}", pinMapCurrent);
    iTxBuffer = (len > 0 ? len : 0);
    Serial.println("ReadGPIO succeeded");
    Serial.println(iTxBuffer);
    Serial.println(jsonTxBuffer);
    interrupts();
    lastReportTimestamp = millis();
    return true;
  }

  return false;
}
*/

void blinkLED() {
  static unsigned long lastBlinkTimestamp = 0;
  static bool bBlink = false;

  if (millis() - lastBlinkTimestamp > 500) {
    digitalWrite(LED_PIN, bBlink ? HIGH : LOW);
    bBlink = !bBlink;
    lastBlinkTimestamp = millis();
  }
}

bool saveStatus() {
  File file = LittleFS.open("/status.dat", "w");
  if (!file) return false;
  file.write((uint8_t*)&servosStatus, sizeof(Servo) * SERVO_COUNT);
  file.close();
  return true;
}

bool loadStatus() {
  File file = LittleFS.open("/status.dat", "r");
//  if (!file || file.size() != sizeof(levelAccel)) return false;
  if (!file) return false;
  file.read((uint8_t*)&servosStatus, sizeof(Servo) * SERVO_COUNT);
  file.close();
  return true;
}

void setServos() {
  for (int i = 0; i < SERVO_COUNT; i++) {
    if (SERVOS_IN_USE_MASK & (1 << i)) {
      pwm.writeMicroseconds(i, servosStatus[i].pulse);
    }
  }
}

void updateServos() {
  static unsigned long nextSaveTimestamp = 0;
  bool changed = false;

  if (iTxBuffer != 0) {
    return;
  }
  
  for (int i = 0; i < SERVO_COUNT; i++) {
    if (SERVOS_IN_USE_MASK & (1 << i)) {
      if (servosStatus[i].pulse != servosStatus[i].targetPulse) {
        changed = true;
        if (abs(servosStatus[i].pulse - servosStatus[i].targetPulse) < SERVO_CHANGE_RATE) {
          servosStatus[i].pulse = servosStatus[i].targetPulse;
          continue;
        }
        servosStatus[i].pulse += ((servosStatus[i].pulse < servosStatus[i].targetPulse) ? SERVO_CHANGE_RATE : -1 * SERVO_CHANGE_RATE);
      }
    }
  }

  if (changed) {
    setServos();
    nextSaveTimestamp = millis() + STATUS_SAVE_DELAY;
    return;
  }

  if (nextSaveTimestamp != 0 && (millis() > nextSaveTimestamp)) {
    nextSaveTimestamp = 0;
    if (saveStatus()) {
      txMessage("{\"info\":\"Status saved\"}");
      Serial.println("Status saved");
    } else {
      txMessage("{\"info\":\"Save status failed\"}");
      Serial.println("Save status failed");
    }
  }
}

AirRide airRide;
Gps gps;

void setup() {
  Serial.begin(115200);  
  pinMode(LED_PIN, OUTPUT);

  airRide.init();
  gps.init();
//  softSerial.begin(9600);

  Wire.setSDA(SDA_PIN_HOST);
  Wire.setSCL(SCL_PIN_HOST);
  Wire.begin(I2C_SLAVE_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Wire1.setSDA(SDA_PIN_SERVO);
  Wire1.setSCL(SCL_PIN_SERVO);
  Wire1.begin();
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);

  for (int i = 0; i < sizeof(inputPins); i++) {
    pinMode(inputPins[i], INPUT);
  }
  for (int i = 0; i < sizeof(outputPins); i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], HIGH);
  }

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  if (!loadStatus()) {
    txMessage("{\"info\":\"No saved status, using defaults\"}");
    for (int i = 0; i < SERVO_COUNT; i++) {
      servosStatus[i].pulse = (SERVOMIN + SERVOMAX) / 2;
      servosStatus[i].targetPulse  = servosStatus[i].pulse;
    }
  } else {
    txMessage("{\"info\":\"Status loaded from flash\"}");
  }

  setServos();
}

void roundRobin() {
  static unsigned long lastTransitionTimestamp = 0;
  static uint8_t i = 0;
  bool fTransition = false;

  airRide.process();
  gps.process();

  if (iTxBuffer != 0 || (millis() - lastTransitionTimestamp < 300)) {
    return;
  }

  switch (i)
  {
  case 0:
    if (airRide.ready()) {
      fTransition = true;
      // Serial.println("roundRobin: airRide ready");
      airRide.send();
    }
    break;
  case 1:
    // Serial.println("roundRobin: readGPIO");
    readGPIO();
    fTransition = true;
    break;
  case 2:
    if (gps.ready()) {
      // Serial.println("roundRobin: gps ready");
      gps.send();
      fTransition = true;
    }
    break;
  default:
    break;
  }

  if (fTransition || (millis() - lastTransitionTimestamp > 500)) {
    i = (i + 1) % 3;
    lastTransitionTimestamp = millis();
  }
}


void loop() {
  parseJsonRxBuffer();
//  readAirRide();
//  readGPIO();
//  readGPS();
  roundRobin();
  updateServos();
  blinkLED();
}
