
/* 
Wayne Manion
April 2017

NodeMCU 1.0 (ESP-12E Module), 80 MHz, 115200, 4M (3M SPIFFS)

--------------------------------------------------------------------------------
Modes:
  0: Out of box experience. Starts an AP and waits for parameters.
  1: Connects in station mode. Changes to configuration not permitted.
     MQTT active if specified.
  2: Connects in station mode and starts an AP. Most functions and config 
     updates are only allowed from AP clients. MQTT active if specified.
  9: Starts in AP and waits for instructions. Reboots into mode 1 after 30 
     minutes. 
--------------------------------------------------------------------------------
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "FS.h"
#define BUFFSIZE 3000
#define VARSIZE 64

// unique
#define SENSE D2
#define SWITCH D1
#define LED LED_BUILTIN

static char *buffer = (char*) malloc(BUFFSIZE), 
  *form  = (char*) malloc(BUFFSIZE / 2),
  *stationSSID = (char*) malloc(VARSIZE + 1), 
  *stationPass = (char*) malloc(VARSIZE + 1), 
  *deviceName = (char*) malloc(VARSIZE + 1),
  *apPass = (char*) malloc(VARSIZE + 1),
  *inTopic = (char*) malloc(VARSIZE + 1),
  *outTopic = (char*) malloc(VARSIZE + 1),
  *mqttBroker = (char*) malloc(VARSIZE + 1),
  *mqttUser = (char*) malloc(VARSIZE + 1),
  *mqttPass = (char*) malloc(VARSIZE + 1);
boolean useMQTT = false;
byte deviceMode = 0, onFail = 0;
int mqttPort = 1883, mqttTimeout = 0;

WiFiClient espClient;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
PubSubClient psClient(espClient);

void setup() {
  initializePins();
  Serial.begin(115200);
  handleDelay(3000);
  Serial.println(F("\n\nSerial communications initialized."));
  jsonSettings();
  startWireless();

  if (deviceMode == 0)
    mode0Setup();
  if (deviceMode == 1)
    mode1Setup();
  if (deviceMode == 2)
    mode2Setup();
  if (deviceMode == 9)
    mode9Setup();

  setPages();
  mqttSetup();
}

void mode0Setup() {
  Serial.println("Initial setup mode initiated...");  
} // end mode0Setup()

void mode1Setup() {
  Serial.println("Station mode initiated.");
} // end mode1Setup()

void mode2Setup() {
  Serial.println("Permanent AP mode initiated.");
} // end mode2Setup()

void mode9Setup() {
  Serial.println("Temporary AP mode initiated.");
} // end mode9Setup()

void loop() {
  // Reboot once per week.
  if (millis() > 7 * 24 * 60 * 60 * 1000);
  server.handleClient();

  if (deviceMode == 1)
    mode1Loop();
  else if (deviceMode == 2)
    mode2Loop();
  else if (deviceMode == 9)
    mode9Loop();
} // end loop()

void mode0Loop() {
  digitalWrite(LED, LOW);
} // end mode0Loop()
	
void mode1Loop() {
  if (!psClient.connected())
    reconnect();
  psClient.loop();
} // end mode1Loop()

void mode2Loop() {
} // end mode2Loop()

void mode9Loop() {
  static unsigned long int startTime = millis();
  // Wait one hour, then revert to previous device mode.
  if (timeDiffNow(startTime) < 60 * 60 * 1000)
    return;
  Serial.println("Advancing to final stage of mode9Loop.");
  deviceMode = 1;
  saveDeviceMode();
  reboot();
} // end mode9Loop()

