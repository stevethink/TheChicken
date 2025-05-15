#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// The TinyGPS++ object
TinyGPSPlus gps;

#define I2C_SLAVE_ADDRESS 0x42
#define SDA_PIN_HOST 0
#define SCL_PIN_HOST 1

// Create a PCA9685 instance on the default I2C address 0x40.
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire1);

#define SDA_PIN_SERVO 2
#define SCL_PIN_SERVO 3
#define SERVOMIN  400 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  2000 // this is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 60 // Analog servos run at ~60 Hz updates
#define SERVO_RANGE 1000

const size_t bufferSize = 1024;
char jsonRxBuffer[bufferSize];
uint16_t iRxBuffer = 0;
char jsonTxBuffer[bufferSize];
uint16_t iTxBuffer = 0;

const uint8_t inputPins[] = {22, 21, 20, 19, 26, 18, 17, 16};
uint8_t pinMapCurrent = 0;

const uint8_t outputPins[] = {13, 12, 11, 10};

int denormalizePulse(int pulse) {
  return map(pulse, 0, SERVO_RANGE, SERVOMIN, SERVOMAX);
}

void txMessage(const char * msg) {
  noInterrupts();
  int len = sprintf(jsonTxBuffer, "%s", msg);
  iTxBuffer = len > 0 ? len : 0;
  interrupts();
}

void parseJsonRxBuffer() {
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
    noInterrupts();
    int len = sprintf(jsonTxBuffer, "{\"error\":\"%s\"}", error.f_str());
    iTxBuffer = len > 0 ? len : 0;
    interrupts();
    return;
  }

  if (doc.containsKey("servo")) {
    if (!doc["servo"].containsKey("num")) {
      txMessage("{\"error\":\"missing servo num value\"}");
      return;
    }
    uint8_t servoNum = doc["servo"]["num"];
    if (servoNum > 15) {
      txMessage("{\"error\":\"servo num out of range\"}");
      return;
    }
    if (!doc["servo"].containsKey("pulse")) {
      txMessage("{\"error\":\"missing servo pulse value\"}");
      return;
    }
    int pulse = doc["servo"]["pulse"];
    if (pulse < 0 || pulse > SERVO_RANGE) {
      txMessage("{\"error\":\"servo pulse out of range\"}");
      return;
    }
    pwm.writeMicroseconds(servoNum, denormalizePulse(pulse));
    txMessage("{\"servo\":\"ack\"}");
  }
  else if (doc.containsKey("relay")) {
    if (!doc["relay"].containsKey("num")) {
      txMessage("{\"error\":\"missing relay num value\"}");
      return;
    }
    uint8_t relayNum = doc["relay"]["num"].as<uint8_t>();
    if (relayNum >= sizeof(outputPins)) {
      txMessage("{\"error\":\"relay num out of range\"}");
      return;
    }
    if (!doc["relay"].containsKey("state")) {
      txMessage("{\"error\":\"missing relay state value\"}");
      return;
    }
    if (doc["relay"]["state"] != "on" && doc["relay"]["state"] != "off") {
      txMessage("{\"error\":\"invalid value for relay state\"}");
      return;
    }

    digitalWrite(outputPins[relayNum], doc["relay"]["state"] == "on" ? LOW : HIGH);
    txMessage("{\"relay\":\"ack\"}");
  }
  else if (doc.containsKey("ping")) {
    txMessage("{\"ping\":\"ack\"}");
    // force resending of current pinMap
    pinMapCurrent = 0xFF;
  }
}

/*
void readCommands() {
  uint32_t status = i2c_get_hw(I2C_INST)->intr_stat;

  // Handle receive data
  if (status & I2C_IC_INTR_STAT_R_RX_FULL_BITS) {
    if (iRxBuffer < bufferSize) {
      char c = (char)i2c_read_byte_raw(I2C_INST);
      if (c == '\n' || c == '\r' || c == '\0') {
        if (iRxBuffer == 0) {
          continue;
        }
        jsonRxBuffer[iRxBuffer] = '\0';
        Serial.println(jsonRxBuffer);
        parseJsonRxBuffer();
        iRxBuffer = 0;
        break;
      }
      if (iRxBuffer >= bufferSize) {
        iRxBuffer = 0;
        break;
      }
      jsonRxBuffer[iRxBuffer++] = c;
    }
  }
  i2c_get_hw(I2C_INST)->clr_rx_full = 1;
}
*/

void receiveEvent(int howMany) {
  while (Wire.available() > 0) {
    char c = (char) Wire.read();
    if (c == '\n' || c == '\r' || c == '\0') {
      if (iRxBuffer == 0) {
        continue;
      }
      jsonRxBuffer[iRxBuffer] = '\0';
      // Serial.println(jsonRxBuffer);
      // parseJsonRxBuffer();
      // iRxBuffer = 0;
      break;
    }
    if (iRxBuffer >= bufferSize) {
      iRxBuffer = 0;
      break;
    }
    jsonRxBuffer[iRxBuffer++] = c;
  }
}

void requestEvent() {
  for (uint16_t i = 0; i < iTxBuffer; i++) {
    Wire.write(jsonTxBuffer[i]);
  }
  Wire.write('\0'); // Ensure null terminator is sent
  iTxBuffer = 0;
}

void readGPS() {
  if (iTxBuffer != 0) {
    return;
  }
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    gps.encode(c);
    if (gps.location.isUpdated()){
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
      iTxBuffer = serializeJson(gpsDoc, jsonTxBuffer, bufferSize);
      return;
      // Serial.println();
//      serializeJson(gpsDoc, Serial);
//      Serial.println();
    }
  }
}

void readGPIO() {
  static unsigned long lastReportTimestamp = 0;
  uint8_t pinMapPrevious = pinMapCurrent;

  if (iTxBuffer != 0) {
    return;
  }

  pinMapCurrent = 0;
  for (int i = 0; i < sizeof(inputPins); i++) {
    pinMapCurrent |= (digitalRead(inputPins[i]) == HIGH ? 1 : 0) << i;
  }

  if (pinMapCurrent != pinMapPrevious || millis() - lastReportTimestamp > 1000) {
    noInterrupts();
    int len = sprintf(jsonTxBuffer, "{\"12vInputs\":%d}", pinMapCurrent);
    iTxBuffer = len > 0 ? len : 0;
    interrupts();
    lastReportTimestamp = millis();
  }
//    Serial.print("{\"12vInputs\":\"");
//    Serial.print(pinMapCurrent);
//    Serial.println("\"}");
}

void setup() {
//  Serial.begin(115200);
  Serial2.begin(9600);

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
}

void loop() {
  parseJsonRxBuffer();
  delay(10);
  readGPS();
  delay(10);
  readGPIO();
  delay(10);
}
