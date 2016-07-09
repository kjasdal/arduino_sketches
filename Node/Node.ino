/*
  Node.cpp - Generic sensor node for use on NodeMCU v1.0 hardware
  Created by Kjetil Asdal, April 1, 2016
*/

#include <Arduino.h>
#include <pins_arduino.h>
#include <EEPROM.h>
#include <Homie.h>
#define DEBUG_SENSOR
#include <Sensor.h>
#include <Bounce2.h>
#include <DHT.h>

// -- DEFINES ------------------------------------------------------------------

#define BOOT_COUNT_SENSOR
#define WIFI_COUNT_SENSOR
#define MQTT_COUNT_SENSOR

#define CONTACT_SENSOR
#define CONTACT_SENSOR_PIN D1
#define CONTACT_SENSOR_DEBOUNCE

#define TEMPERATURE_SENSOR
#define TEMPERATURE_SENSOR_HYSTERESIS 1.00
#define HUMIDITY_SENSOR
#define HUMIDITY_SENSOR_HYSTERESIS 1.00
#define DHT_PIN D2
#define DHTTYPE DHT11

#define LIGHT_SENSOR
#define LIGHT_SENSOR_PIN A0
//#define LIGHT_SENSOR_CONTINUOUS
//#define LIGHT_SENSOR_HYSTERESIS 10
#define LIGHT_SENSOR_BINARY
#define LIGHT_SENSOR_THRESHOLD 768

//#define MOTION_SENSOR
#define MOTION_SENSOR_PIN D5

// -- PRIVATE VARIABLES --------------------------------------------------------

#ifdef BOOT_COUNT_SENSOR
static byte bootCount;
static Sensor bootCountSensor;
#endif

#ifdef WIFI_COUNT_SENSOR
static unsigned int wifiCount;
static Sensor wifiCountSensor;
#endif

#ifdef MQTT_COUNT_SENSOR
static unsigned int mqttCount;
static Sensor mqttCountSensor;
#endif

#if defined(BOOT_COUNT_SENSOR) || defined(WIFI_COUNT_SENSOR) || defined(MQTT_COUNT_SENSOR)
static HomieNode systemNode("system", "system");
#endif

#ifdef CONTACT_SENSOR
#ifdef CONTACT_SENSOR_DEBOUNCE
static Bounce contactSensorBounce;
#endif
static Sensor contactSensor;
static HomieNode contactNode("contact", "contact");
#endif

#if defined(TEMPERATURE_SENSOR) || defined(HUMIDITY_SENSOR)
static DHT dht(DHT_PIN, DHTTYPE);
#ifdef TEMPERATURE_SENSOR
static Sensor temperatureSensor;
static HomieNode temperatureNode("temperature", "temperature");
#endif
#ifdef HUMIDITY_SENSOR
static Sensor humiditySensor;
static HomieNode humidityNode("humidity", "humidity");
#endif
#endif

#ifdef LIGHT_SENSOR
static Sensor lightSensor;
static HomieNode lightNode("light", "light");
#endif

#ifdef MOTION_SENSOR
static Sensor motionSensor;
static HomieNode motionNode("motion", "motion");
#endif

// -- PRIVATE FUNCTION PROTOTYPES ----------------------------------------------

static void setupHandler();
static void loopHandler();
static void onHomieEvent(HomieEvent event);

#ifdef BOOT_COUNT_SENSOR
static void bootCountSensorReadStateCB(String &state);
static void bootCountSensorPostStateCB(String state);
#endif

#ifdef WIFI_COUNT_SENSOR
static void wifiCountSensorReadStateCB(String &state);
static void wifiCountSensorPostStateCB(String state);
#endif

#ifdef MQTT_COUNT_SENSOR
static void mqttCountSensorReadStateCB(String &state);
static void mqttCountSensorPostStateCB(String state);
#endif

#ifdef CONTACT_SENSOR
static void contactSensorReadStateCB(String &state);
static void contactSensorPostStateCB(String state);
#endif

#ifdef TEMPERATURE_SENSOR
static void temperatureSensorReadStateCB(String &state);
static int temperatureSensorCompareStateCB(String state, String previousState);
static void temperatureSensorPostStateCB(String state);
#endif

#ifdef HUMIDITY_SENSOR
static void humiditySensorReadStateCB(String &state);
static int humiditySensorCompareStateCB(String state, String previousState);
static void humiditySensorPostStateCB(String state);
#endif

#ifdef LIGHT_SENSOR
static void lightSensorReadStateCB(String &state);
static int lightSensorCompareStateCB(String state, String previousState);
static void lightSensorPostStateCB(String state);
#endif

#ifdef MOTION_SENSOR
static void motionSensorReadStateCB(String &state);
static int motionSensorCompareStateCB(String state, String previousState);
static void motionSensorPostStateCB(String state);
#endif

// -- PUBLIC FUNCTIONS -----------------------------------------------------------

void setup() {
#ifdef BOOT_COUNT_SENSOR
  // -- Increase boot count --
  EEPROM.begin(512);
  bootCount = EEPROM.read(0) + 1;
  EEPROM.write(0, bootCount);
  EEPROM.commit();
  EEPROM.end();
  // ----------- O -----------
#endif

#ifdef WIFI_COUNT_SENSOR
  wifiCount = 0;
#endif

#ifdef MQTT_COUNT_SENSOR
  mqttCount = 0;
#endif

  Homie.setFirmware("Node", "0.1.0");

/* Apparently a call to .registerNode() is no longer needed

#if defined(BOOT_COUNT_SENSOR) || defined(WIFI_COUNT_SENSOR) || defined(MQTT_COUNT_SENSOR)
  Homie.registerNode(systemNode);
#endif
#ifdef CONTACT_SENSOR  
  Homie.registerNode(contactNode);
#endif
#ifdef TEMPERATURE_SENSOR
  Homie.registerNode(temperatureNode);
#endif
#ifdef HUMIDITY_SENSOR
  Homie.registerNode(humidityNode);
#endif
#ifdef LIGHT_SENSOR
  Homie.registerNode(lightNode);
#endif
#ifdef MOTION_SENSOR
  Homie.registerNode(motionNode);
#endif
*/

  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  Homie.setup();
}

void loop() {
  Homie.loop();
}

// -- PRIVATE FUNCTIONS ----------------------------------------------------------

static void setupHandler() {
#ifdef BOOT_COUNT_SENSOR
  bootCountSensor.setReadInterval(15 * 1000);
  bootCountSensor.setPostInterval(60 * 1000);
  bootCountSensor.setReadStateCB(bootCountSensorReadStateCB);
  bootCountSensor.setPostStateCB(bootCountSensorPostStateCB);
#endif

#ifdef WIFI_COUNT_SENSOR
  wifiCountSensor.setReadInterval(15 * 1000);
  wifiCountSensor.setPostInterval(60 * 1000);
  wifiCountSensor.setReadStateCB(wifiCountSensorReadStateCB);
  wifiCountSensor.setPostStateCB(wifiCountSensorPostStateCB);
#endif

#ifdef MQTT_COUNT_SENSOR
  mqttCountSensor.setReadInterval(15 * 1000);
  mqttCountSensor.setPostInterval(60 * 1000);
  mqttCountSensor.setReadStateCB(mqttCountSensorReadStateCB);
  mqttCountSensor.setPostStateCB(mqttCountSensorPostStateCB);
#endif
  
#ifdef CONTACT_SENSOR
  pinMode(CONTACT_SENSOR_PIN, INPUT_PULLUP);
#ifdef CONTACT_SENSOR_DEBOUNCE
  contactSensorBounce.attach(CONTACT_SENSOR_PIN);
  contactSensorBounce.interval(5);
#endif
  contactSensor.setReadInterval(1 * 1000);
  contactSensor.setPostInterval(60 * 1000);
  contactSensor.setReadStateCB(contactSensorReadStateCB);
  contactSensor.setPostStateCB(contactSensorPostStateCB);
#endif

#if defined(TEMPERATURE_SENSOR) || defined(HUMIDITY_SENSOR)
  dht.begin();
#ifdef TEMPERATURE_SENSOR
  temperatureSensor.setReadInterval(15 * 1000);
  temperatureSensor.setPostInterval(60 * 1000);
  temperatureSensor.setReadStateCB(temperatureSensorReadStateCB);
  temperatureSensor.setCompareStateCB(temperatureSensorCompareStateCB);
  temperatureSensor.setPostStateCB(temperatureSensorPostStateCB);
#endif

#ifdef HUMIDITY_SENSOR
  humiditySensor.setReadInterval(20 * 1000);
  humiditySensor.setPostInterval(60 * 1000);
  humiditySensor.setReadStateCB(humiditySensorReadStateCB);
  humiditySensor.setCompareStateCB(humiditySensorCompareStateCB);
  humiditySensor.setPostStateCB(humiditySensorPostStateCB);
#endif
#endif

#ifdef LIGHT_SENSOR
  lightSensor.setReadInterval(1 * 1000);
  lightSensor.setPostInterval(60 * 1000);
  lightSensor.setReadStateCB(lightSensorReadStateCB);
#ifdef LIGHT_SENSOR_CONTINUOUS
  lightSensor.setCompareStateCB(lightSensorCompareStateCB);
#endif
  lightSensor.setPostStateCB(lightSensorPostStateCB);
#endif

#ifdef MOTION_SENSOR
  pinMode(MOTION_SENSOR_PIN, INPUT);
  motionSensor.setReadInterval(1 * 1000);
  motionSensor.setPostInterval(60 * 1000);
  motionSensor.setReadStateCB(motionSensorReadStateCB);
  motionSensor.setPostStateCB(motionSensorPostStateCB);
#endif
}

static void loopHandler() {
#ifdef BOOT_COUNT_SENSOR
  bootCountSensor.loop();
#endif
#ifdef WIFI_COUNT_SENSOR
  wifiCountSensor.loop();
#endif
#ifdef MQTT_COUNT_SENSOR
  mqttCountSensor.loop();
#endif
#ifdef CONTACT_SENSOR
  contactSensor.loop();
#endif
#ifdef TEMPERATURE_SENSOR  
  temperatureSensor.loop();
#endif
#ifdef HUMIDITY_SENSOR  
  humiditySensor.loop();
#endif
#ifdef LIGHT_SENSOR
  lightSensor.loop();
#endif
#ifdef MOTION_SENSOR
  motionSensor.loop();
#endif
}

static void onHomieEvent(HomieEvent event) {
  switch(event) {
#ifdef WIFI_COUNT_SENSOR    
    case HOMIE_WIFI_CONNECTED:
      wifiCount++;
      break;
#endif
#ifdef MQTT_COUNT_SENSOR    
    case HOMIE_MQTT_CONNECTED:
      mqttCount++;
      break;
#endif
    default:
      break;
  }
}

#ifdef BOOT_COUNT_SENSOR
static void bootCountSensorReadStateCB(String &state) {
  state = bootCount;
}
#endif

#ifdef BOOT_COUNT_SENSOR
static void bootCountSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(systemNode, "boot", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef WIFI_COUNT_SENSOR
static void wifiCountSensorReadStateCB(String &state) {
  state = wifiCount;
}
#endif

#ifdef WIFI_COUNT_SENSOR
static void wifiCountSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(systemNode, "wifi", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef MQTT_COUNT_SENSOR
static void mqttCountSensorReadStateCB(String &state) {
  state = mqttCount;
}
#endif

#ifdef MQTT_COUNT_SENSOR
static void mqttCountSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(systemNode, "mqtt", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef CONTACT_SENSOR
static void contactSensorReadStateCB(String &state) {
#ifdef CONTACT_SENSOR_DEBOUNCE
  contactSensorBounce.update();
  state = contactSensorBounce.read(); 
#else
  state = digitalRead(CONTACT_SENSOR_PIN); 
#endif
}
#endif

#ifdef CONTACT_SENSOR
static void contactSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(contactNode, "contact", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef TEMPERATURE_SENSOR
static void temperatureSensorReadStateCB(String &state) {
  float t = dht.readTemperature();
  if (isnan(t)) {
    state = ""; // Invalid read
  }
  else {
    state = t;
  }
}
#endif

#ifdef TEMPERATURE_SENSOR
static int temperatureSensorCompareStateCB(String state, String previousState) {
  if ((state.toFloat() < (previousState.toFloat() - TEMPERATURE_SENSOR_HYSTERESIS)) || 
     (state.toFloat() > (previousState.toFloat() + TEMPERATURE_SENSOR_HYSTERESIS))) {
    return 1; 
  }
  return 0;
}
#endif

#ifdef TEMPERATURE_SENSOR
static void temperatureSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(temperatureNode, "temperature", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef HUMIDITY_SENSOR
static void humiditySensorReadStateCB(String &state) {
  float h = dht.readHumidity();
  if (isnan(h)) {
    state = ""; // Invalid read
  }
  else {
    state = h;
  }
}
#endif

#ifdef HUMIDITY_SENSOR
static int humiditySensorCompareStateCB(String state, String previousState) {
  if ((state.toFloat() < (previousState.toFloat() - HUMIDITY_SENSOR_HYSTERESIS)) || 
     (state.toFloat() > (previousState.toFloat() + HUMIDITY_SENSOR_HYSTERESIS))) {
    return 1; 
  }
  return 0;
}
#endif

#ifdef HUMIDITY_SENSOR
static void humiditySensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(humidityNode, "humidity", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef LIGHT_SENSOR
static void lightSensorReadStateCB(String &state) {
#ifdef LIGHT_SENSOR_CONTINUOUS
  state = analogRead(LIGHT_SENSOR_PIN);
#endif
#ifdef LIGHT_SENSOR_BINARY
  // OPEN = Light, CLOSED = Dark
//  state = (analogRead(LIGHT_SENSOR_PIN) < LIGHT_SENSOR_THRESHOLD) ? "OPEN" : "CLOSED";
  state = (analogRead(LIGHT_SENSOR_PIN) < LIGHT_SENSOR_THRESHOLD);
#endif
}
#endif

#ifdef LIGHT_SENSOR
#ifdef LIGHT_SENSOR_CONTINUOUS
static int lightSensorCompareStateCB(String state, String previousState) {
  if ((state.toInt() < (previousState.toInt() - LIGHT_SENSOR_HYSTERESIS)) || 
     (state.toInt() > (previousState.toInt() + LIGHT_SENSOR_HYSTERESIS))) {
    return 1; 
  }
  return 0;
}
#endif
#endif

#ifdef LIGHT_SENSOR
static void lightSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(lightNode, "light", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif

#ifdef MOTION_SENSOR
static void motionSensorReadStateCB(String &state) {
  state = digitalRead(MOTION_SENSOR_PIN); 
}
#endif

#ifdef MOTION_SENSOR
static void motionSensorPostStateCB(String state) {
  if (!Homie.setNodeProperty(motionNode, "motion", state, true)) {
    Serial.println("Sending failed");
  }
}
#endif
