boolean saveDeviceMode() {
  char fileName[] = "/deviceMode.json";
  SPIFFS.begin();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["deviceMode"] = deviceMode;
  
  File jsonFile = SPIFFS.open(fileName, "w");
  if (!jsonFile) {
    Serial.print(F("Failed to open "));
    Serial.print(fileName);
    Serial.println(F(" for writing."));
    SPIFFS.end();
    return false;
  }

  json.printTo(jsonFile);
  Serial.print(F("Wrote file:"));
  Serial.println(fileName);
  SPIFFS.end();
  return true;
} // end deviceMode()

boolean saveStationConfig() {
  char fileName[] = "/station.json";
  SPIFFS.begin();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["stationSSID"] = stationSSID;
  json["stationPass"] = stationPass;
  
  File jsonFile = SPIFFS.open(fileName, "w");
  if (!jsonFile) {
    Serial.print(F("Failed to open "));
    Serial.print(fileName);
    Serial.println(F(" for writing."));
    SPIFFS.end();
    return false;
  }

  json.printTo(jsonFile);
  Serial.print(F("Wrote file:"));
  Serial.println(fileName);
  SPIFFS.end();
  return true;
} // end saveStationConfig()

boolean saveAPConfig() {
  char fileName[] = "/ap.json";
  SPIFFS.begin();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["apPass"] = apPass;
  json["deviceName"] = deviceName;
  
  File jsonFile = SPIFFS.open(fileName, "w");
  if (!jsonFile) {
    Serial.print(F("Failed to open "));
    Serial.print(fileName);
    Serial.println(F(" for writing."));
    SPIFFS.end();
    return false;
  }

  json.printTo(jsonFile);
  Serial.print(F("Wrote file: "));
  Serial.println(fileName);
  SPIFFS.end();
  if (deviceMode == 0) {
    deviceMode = 2;
    saveDeviceMode();
  }
  return true;
} // end saveAPConfig()

boolean saveMQTTConfig() {
  char fileName[] = "/mqtt.json";
  char temp[6] = "false";
  SPIFFS.begin();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  if (useMQTT)
    strcpy(temp, "true");
  json["useMQTT"] = temp;
  json["mqttBroker"] = mqttBroker;
  json["mqttPass"] = mqttPass;
  json["mqttUser"] = mqttUser;
  json["inTopic"] = inTopic;
  json["outTopic"] = outTopic;
  json["mqttPort"] = mqttPort;
  json["mqttTimeout"] = mqttTimeout;
  json["onFail"] = onFail;
  
  File jsonFile = SPIFFS.open(fileName, "w");
  if (!jsonFile) {
    Serial.print(F("Failed to open "));
    Serial.print(fileName);
    Serial.println(F(" for writing."));
    SPIFFS.end();
    return false;
  }

  json.printTo(jsonFile);
  Serial.print(F("Wrote file: "));
  Serial.println(fileName);
  SPIFFS.end();
  return true;
} // end saveMQTTConfig()

void wipeSpiffs() {
  Serial.println(F("Formatting SPIFFS."));
  SPIFFS.format();
  Serial.println(F("Format complete."));
} // end wipeSpiffs()

void switchMode(byte choice) {
  boolean ok = true;
  if (strlen(stationSSID) == 0)
    ok = false;
  if (choice == 2 && strlen(apPass) == 0)
    ok = false;
  if (deviceMode == choice)
    ok = false;
  if (!ok) {
    Serial.println("Mode change not permitted.");
    page("Permission denied", "Permission denied.\n<p>\n", 0);
    sendMQTT("Mode change not permitted.");
    return;
  }
  
  if (choice == 1)
    page("Changing to station mode.", "Changing to station mode.\n<p>\n", 10);
  if (choice == 2)
    page("Changing to AP mode.", "Changing to AP mode.\n<p>\n", 10);
  if (choice == 9)
    page("Changing to temporary AP temporary mode.", "Changing to temporary AP mode.\n<p>\n", 10);
  
  deviceMode = choice;
  saveDeviceMode();
  Serial.print("Booting in mode: ");
  Serial.println(deviceMode);
  handleDelay(500);
  ESP.restart();
} // end switchMode()

