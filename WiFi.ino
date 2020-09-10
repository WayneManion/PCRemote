void startWireless() {
  startWireless(120);
} // end startWireless()

void startWireless(int howLong) {
  WiFi.mode(WIFI_OFF);
  if (deviceMode == 1) {
    if (!startStation(howLong))
      switchMode(9);
  }
  if (deviceMode != 1)
    startAP();
} // end startWireless()

void startAP() {
  handleDelay(200);
  if (deviceMode == 2 || deviceMode == 9)
    WiFi.mode(WIFI_AP_STA);
  else
    WiFi.mode(WIFI_AP);
  if (strlen(apPass) >= 8)
    WiFi.softAP(deviceName, apPass);
  else {
    if (deviceMode == 0)
      WiFi.softAP(deviceName);
    else
      ESP.restart();
  }
  handleDelay(10);
  Serial.print("Setting up AP: ");
  Serial.print(deviceName);
  Serial.print(" with IP Address: ");
  Serial.println(WiFi.softAPIP());
} // end startAP()

boolean startStation(int howLong) {
  if (deviceMode == 1)
    WiFi.mode(WIFI_STA);
  else
    WiFi.mode(WIFI_AP_STA);
  WiFi.hostname(deviceName);
  Serial.print(F("Attempting connection to SSID: "));
  Serial.print(stationSSID);
  Serial.print(F(" with hostname: "));
  Serial.println(deviceName);
  WiFi.begin(stationSSID, stationPass);

  // Try to connect for specified time, then give up.
  for (int i = 0; i < howLong; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      handleDelay(1000);
      Serial.print(F("."));
    }
    // If connection achieved, write to serial and return true.
    else {
      Serial.print(F("\nConnected to SSID: "));
      Serial.print(stationSSID);
      Serial.print(F(" with IP address: "));
      Serial.println(WiFi.localIP());
      return true;
    }
  }
  // Return false if connection not achieved.
  Serial.print(F("\nCould not connect to SSID: "));
  Serial.println(stationSSID);
  WiFi.mode(WIFI_OFF);
  return false;
} // end startStationMode()
