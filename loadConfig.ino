void jsonSettings() {
  deviceMode = loadDeviceMode();
  loadAPConfig();
  loadStationConfig();
  loadMQTTConfig();
} // end jsonSettings()

char loadDeviceMode() {
  char temp = atoi(loadItem("deviceMode", "/deviceMode.json"));
  return temp;
}

boolean loadStationConfig() {
  char fileName[] = "/station.json";
  strcpy(stationSSID, loadItem("stationSSID", fileName));
  if (stationSSID[0] == 0)
    return false;
  strcpy(stationPass, loadItem("stationPass", fileName));
  return true;
} // end loadStationConfig()

boolean loadAPConfig() {
  char fileName[] = "/ap.json";
  strcpy(apPass, loadItem("apPass", fileName));
  strcpy(deviceName, loadItem("deviceName", fileName));
  if (deviceName[0] == 0)
    strcpy(deviceName, generateHostName());
  if (apPass[0] == 0)
    return false;
  return true;
} // end loadAPSettings()

boolean loadMQTTConfig() {
  char temp[] = "false";
  char fileName[] = "/mqtt.json";

  strcpy(temp, loadItem("useMQTT", fileName));
  if (strcmp(temp, "true") == 0)
    useMQTT = true;
  strcpy(mqttBroker, loadItem("mqttBroker", fileName));
  strcpy(mqttUser, loadItem("mqttUser", fileName));
  strcpy(mqttPass, loadItem("mqttPass", fileName));
  strcpy(inTopic, loadItem("inTopic", fileName));
  strcpy(outTopic, loadItem("outTopic", fileName));
  mqttPort = atoi(loadItem("mqttPort", fileName));
  mqttTimeout = atoi(loadItem("mqttTimeout", fileName));
  onFail = atoi(loadItem("onFail", fileName));
  
  if (mqttPort < 1 || mqttPort > 65536)
    mqttPort = 1883;
  
  if (mqttBroker[0] == 0 || inTopic[0] == 0 || outTopic[0] == 0)
    suspendMQTT();
  
  return true;
} // end loadMQTTConfig()

char* loadItem(const char* itemName, const char* fileName) {
  SPIFFS.begin();
  char temp[VARSIZE + 1] = "";
  File jsonFile = SPIFFS.open(fileName, "r");
  if (!jsonFile) {
    Serial.print("File ");
    Serial.print(fileName);
    Serial.println(" not found.");
    SPIFFS.end();
    return temp;
  }

  size_t size = jsonFile.size();
  if (size > 1024) {
    Serial.print(F("File "));
    Serial.print(fileName);
    Serial.println((" is too large."));
    SPIFFS.end();
    return temp;
  }
  
  std::unique_ptr<char[]> buf(new char[size]);
  jsonFile.readBytes(buf.get(), size);
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  
  if (!json.success()) {
    Serial.print("Failed to parse file ");
    Serial.print(fileName);
    Serial.println(".");
    SPIFFS.end();
    return temp;
  }

  if (!json.containsKey(itemName)) {
    Serial.print(itemName);
    Serial.print(" not present in ");
    Serial.print(fileName);
    Serial.println(".");
    SPIFFS.end();
    return temp;
  }
  
  strncpy(temp, json[itemName], VARSIZE);
  SPIFFS.end();

  Serial.print("Loaded ");
  Serial.print(itemName);
  Serial.print(": ");
  if (strstr(itemName, "Pass") > 0)
    printStars(temp);
  else
    Serial.print(temp);
  Serial.print(" from file: ");
  Serial.println(fileName);
  return temp;
} // end loadItem()

