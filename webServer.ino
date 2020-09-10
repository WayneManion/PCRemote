void setPages() {
  server.on("/css.css", css);
  server.on("/reboot", reboot);
  if (deviceMode == 0) {
    server.on("/apConfigRec", apConfigRec);
    server.on("/factoryReset", factoryReset);
    server.on("/apConfig", apConfig);
    server.onNotFound(apConfig);
  }
  if (deviceMode > 0) {
    server.on("/changeMode", changeMode);
    server.on("/mode1", mode1);
    server.on("/mode2", mode2);
    server.on("/", specialHome);
  }
  if (deviceMode == 1)
    server.onNotFound(specialHome);
  if (deviceMode > 1) {
    server.on("/index", specialHome);
    server.on("/index.htm", specialHome);
    server.on("/index.html", specialHome);
    server.on("/apConfig", apConfig);
    server.on("/apConfigRec", apConfigRec);
    server.on("/stationConfig", stationConfig);
    server.on("/stationConfigRec", stationConfigRec);
    server.on("/mqttConfig", mqttConfig);
    server.on("/mqttConfigRec", mqttConfigRec);
    server.on("/factoryReset", factoryReset);
    httpUpdater.setup(&server);
  }
  
  setUniquePages();
  server.begin();
  Serial.println(F("HTTP server initialized."));
} // end setPages()

void css() {
  server.send(200, "text/css", 
    "body { font-size: 16px; font-family: "
    "sans-serif; text-align: center; } \n"
    "h1 { font-size: 24px; font-weight: normal; } \n"
    "button, input[type=submit] { font-size: 20px; border: 8px;\n"
    "  padding: 12px; width: 324px;\n"
    "  background-color: #A9A9A9; } \n"
    ".shiny, input.shiny { background-color: #B0E0E6; } \n"
    ".dull { background-color: #DCDCDC; } \n"
    "form { width: 324px; margin: 0 auto; } \n"
    "label, input { width: 160px; display: inline-block; "
    "font-size: 16px; margin: 0 0; } \n"
    "label { text-align: right; } \n"
    "input + input { width: 324px; float: right; }");
} // end css()

void reboot() {
  page("Resetting device", "Resetting device<p>\n", 10);
  Serial.println("Resetting device.");
  handleDelay(500);
  ESP.restart();
} // end reboot()

void page(const char* title, const char* body, byte refresh) {
  char temp[8], clientIP[16];
  itoa(refresh, temp, 10);
  strcpy(buffer, "<!DOCTYPE html>\n<html><head>"
    "<link rel='stylesheet' href='css.css'>\n<title>");
  strcpy(clientIP, server.client().remoteIP().toString().c_str());
  strcat(buffer, deviceName);
  strcat(buffer, ": ");
  strcat(buffer, title);
  strcat(buffer, "</title>\n"
    "<meta name='viewport' content='width=device-width,\n"
    "  initial-scale=1.0, maximum-scale=1.0, user-scalable=no' />\n");
  if (refresh > 0) {
    strcat(buffer, "<meta http-equiv='refresh' content='");
    strcat(buffer, temp);
    strcat(buffer, "; url=/'>\n");
  }
  strcat(buffer, "</head><body>\n");
  strcat(buffer, body);
  strcat(buffer, "<a href='/index.html'>"
    "<button>Device home page</button></a><p>\n"
    "<a href='/changeMode'><button>Change device mode</button></a><p>\n");
  strcat(buffer, "<a href='/reboot'><button>Reboot device</button></a><p>\n");
  if (deviceMode > 1)
    strcat(buffer, "<a href='/mqttConfig'><button>Configure MQTT"
      "</button></a><p>\n"
      "<a href='/stationConfig'><button>Configure Wi-Fi station"
      "</button></a><p>\n"
      "<a href='/apConfig'><button>Configure Wi-Fi AP</button></a><p>\n"
      "<a href='/factoryReset'><button class='dull'>"
      "Wipe all settings</button></a><p>\n"
      "<a href='/update'><button class='dull'>"
      "Upload new firmware</button></a><p>\n");
  strcat(buffer, "Client IP address: ");
  strcat(buffer, server.client().remoteIP().toString().c_str());
  if (strlen(stationSSID) != 0) {
    strcat(buffer, "<br>\nStation SSID: ");
    strcat(buffer, stationSSID);
  }
  strcat(buffer, "<br>\nDevice name: ");
  strcat(buffer, deviceName);
  if (useMQTT) {
    strcat(buffer, "<br>\nMQTT enabled.<br>\nMQTT Broker: ");
    strcat(buffer, mqttBroker);
    strcat(buffer, "<br>\nMQTT Port: ");
    itoa(mqttPort, temp, 10);
    strcat(buffer, temp);
    strcat(buffer, "<br>\nMQTT User: ");
    strcat(buffer, mqttUser);
    strcat(buffer, "<br>\nMQTT incoming topic: ");
    strcat(buffer, inTopic);
    strcat(buffer, "<br>\nMQTT outgoing topic: ");
    strcat(buffer, outTopic);
  }
  else
    strcat(buffer, "<br>\nMQTT disabled.");
  if (deviceMode == 0)
    strcat(buffer, "<br>\nDevice in unconfigured mode.");
  else if (deviceMode == 1)
    strcat(buffer, "<br>\nDevice in station mode.");
  else if (deviceMode == 2)
    strcat(buffer, "<br>\nDevice in permanent AP mode.");
  else if (deviceMode == 9)
    strcat(buffer, "<br>\nDevice in temporary AP-only mode.");
  strcat(buffer, "<br>\nDevice MAC ID: ");
  strcat(buffer, WiFi.macAddress().c_str());
  strcat(buffer, "<br>\nFirmware compiled: ");
  strcat(buffer, __DATE__ " " __TIME__);
  strcat(buffer, "<br>\n</body></html>");
  server.send(200, "text/html", buffer);
  Serial.print("Served: ");
  Serial.print(title);
  Serial.print(". Page size: ");
  Serial.print(strlen(buffer));
  Serial.print(" bytes. Free heap size: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes.");
} // end page()


void apConfig() {
  if (deviceMode == 1 || deviceMode == 9)
    return;
  strcpy(form, 
    "<form action='/apConfigRec' method='POST'>"
	  "<h1>Enter new AP configuration.</h1>\n"
    "Password must contain at <br>least eight characters.<p>\n"
    "Device name will define AP SSID <br>and station hostname.<p>\n"
    "<label>Device name:</label>"
	  "<input type='text' name='deviceName' placeholder='");
  strcat(form, deviceName);
  if (strlen(apPass) >= 8)
    strcat(form, "'><p>\n<label>Current AP password:</label>"
      "<input type='password' name='oldPass");
  strcat(form, "'><p>\n"
    "<label>New AP password:</label>"
	  "<input type='password' name='apPass1'><p>\n"
    "<label>Verify new password:</label>"
	  "<input type='password' name='apPass2'><p>\n"
    "<input type='submit' class='shiny' name='SUBMIT' "
    "value='Submit AP configuration'>"
	  "</form>\n<p>\n");
  page("AP configuration", form, 0);
} // end apConfig()

void apConfigRec() {
  char dN[VARSIZE + 1] = "", p1[VARSIZE + 1] = "", p2[VARSIZE + 1] = "", 
    old[VARSIZE + 1] = "", message[VARSIZE * 2 + 1] = "";
  if (deviceMode == 1 || deviceMode == 9)
    return;
  if (server.hasArg("deviceName"))
    strncpy(dN, server.arg("deviceName").c_str(), VARSIZE);
  if (server.hasArg("oldPass"))
    strncpy(old, server.arg("oldPass").c_str(), VARSIZE);
  if (server.hasArg("apPass1"))
    strncpy(p1, server.arg("apPass1").c_str(), VARSIZE);
  if (server.hasArg("apPass2"))
    strncpy(p2, server.arg("apPass2").c_str(), VARSIZE);

  if (strlen(p1) < 8 || strlen(p2) < 8)
    strcat(message, "New password must contain at least 8 characters. ");
  if (strcmp(p1, p2) != 0)
    strcat(message, "New passwords do not match. ");
  if (strcmp(apPass, old) != 0)
    strcat(message, "Invalid current password. ");
    
  
  if (strlen(message) > 0) {
    char temp[129] = "Ignoring all input. ";
    strcat(temp, message);
    strcpy(message, temp);
  }
  else {
    strcpy(message, "AP config input accepted. ");
    strcpy(apPass, p1);
    strcpy(deviceName, dN);
    saveAPConfig();
  }
  Serial.println(message);
  strcat(message, "<p>");
  page("Received AP configuration", message, 10);
  if (strcmp(message, "AP config input accepted. <p>") == 0); {
    Serial.println("Rebooting device with new AP config.");
    handleDelay(500);
    ESP.restart();
  }
} // end apConfigRec()

void stationConfig() {
  if (deviceMode != 2)
    return;
  strcpy(form, 
    "<form action='/stationConfigRec' method='POST'><h1>"
    "Enter new station configuration.</h1>\n"
    "<label>Station SSID:</label>"
    "<input type='text' name='stationSSID' placeholder='");
  strcat(form, stationSSID);
  if (strlen(apPass) >= 8)
    strcat(form, "'><p>\n<label>Current AP password:</label>"
      "<input type='password' name='apPass");
  strcat(form, "'><p>\n"
    "<label>Station password:</label>"
	  "<input type='password' name='stationPass1'><p>\n"
    "<label>Verify new password:</label>"
	  "<input type='password' name='stationPass2'><p>\n"
    "<input type='submit' name='SUBMIT' class='shiny' "
    "value='Submit station configuration'>"
	  "</form>\n<p>\n");
  page("Station configuration", form, 0);
} // end stationConfig()

void stationConfigRec() {
  if (deviceMode != 2)
    return;
  char ss[VARSIZE + 1] = "", p1[VARSIZE + 1] = "", p2[VARSIZE + 1] = "", 
    apP[VARSIZE + 1] = "", message[VARSIZE * 2 + 1] = "";
  if (server.hasArg("stationSSID"))
    strncpy(ss, server.arg("stationSSID").c_str(), VARSIZE);
  if (server.hasArg("apPass"))
    strncpy(apP, server.arg("apPass").c_str(), VARSIZE);
  if (server.hasArg("stationPass1"))
    strncpy(p1, server.arg("stationPass1").c_str(), VARSIZE);
  if (server.hasArg("stationPass2"))
    strncpy(p2, server.arg("stationPass2").c_str(), VARSIZE);

  if (strlen(p1) < 8 || strlen(p2) < 8)
    strcat(message, "New password must contain at least 8 characters. ");
  if (strcmp(p1, p2) != 0)
    strcat(message, "New passwords do not match. ");
  if (strcmp(apPass, apP) != 0)
    strcat(message, "Invalid AP password. ");
    
  
  if (strlen(message) > 0) {
    char temp[VARSIZE * 2 + 1] = "Ignoring all input. ";
    strcat(temp, message);
    strcpy(message, temp);
  }
  else {
    strcpy(message, "Station config input accepted. ");
    strcpy(stationPass, p1);
    strcpy(stationSSID, ss);
    saveStationConfig();
    if (deviceMode > 2) {
      deviceMode = 1;
      saveDeviceMode();
    }
  }
  Serial.println(message);
  strcat(message, "<p>");
  page("Received station configuration", message, 10);
} // end stationConfigRec()

void factoryReset() {
  if (deviceMode == 1)
    return;
  page("<h1>Erasing configuration", 
    "<h1>Erasing all configuration data and rebooting.</h1>", 10);
  Serial.println("Erasing config data.");
  handleDelay(500);
  wipeSpiffs();
  ESP.restart();
} // end factoryReset()

void changeMode() {
  char options[257] = "";;
  if (strlen(stationSSID) > 0)
    strcat(options, "<a href='mode1'>"
      "<button class='shiny'>Station mode</button></a><p>\n");
  if (strlen(stationSSID) > 0 && strlen(apPass) >= 8)
    strcat(options, "<a href='mode2'>"
      "<button class='shiny'>Permanent AP mode</button></a><p>\n");
  strcat(options, "\n<p>\n");
  page("Mode selection", options, 0);
} // end changeMode()

void mode1() {
  switchMode(1);
} // end mode1()

void mode2() {
  switchMode(2);
} // end mode2()

void mqttConfig() {
  if (deviceMode != 2)
    return;
  char temp[6] = "";
  strcpy(form, "<form action='/mqttConfigRec' method='POST'>"
    "<h1>Enter new MQTT configuration.</h1>\n"
    "Current AP password is required.<p>\n");
  if (strlen(apPass) >= 8)
    strcat(form, "<label>Current AP password:</label>"
      "<input type='password' name='apPass'><p>\n");
  strcat(form, "<label>MQTT Broker:</label>"
    "<input type='text' name='mqttBroker' placeholder='");
  strcat(form, mqttBroker);
  strcat(form, "'><p>\n<label>MQTT Port:</label>"
    "<input type='number' name='mqttPort' placeholder='");
  itoa(mqttPort, temp, 10);
  strcat(form, temp);
  strcat(form, "'><p>\n<label>MQTT User:</label>"
    "<input type='text' name='mqttUser' placeholder='");
  strcat(form, mqttUser);  
  strcat(form, "'><p>\n"
    "<label>New MQTT password:</label>"
    "<input type='password' name='mqttPass1'><p>\n"
    "<label>Verify new password:</label>"
	  "<input type='password' name='mqttPass2'><p>\n"
    "<label>MQTT input topic:</label>"
	  "<input type='text' name='inTopic' placeholder='");
  strcat(form, inTopic);
  strcat(form, "'><p>\n<label>MQTT output topic:</label>"
    "<input type='text' name='outTopic' placeholder='");
  strcat(form, outTopic);
  strcat(form, "'><p>\n<label>MQTT Timeout:</label>"
    "<input type='number' name='mqttTimeout' placeholder='");
  itoa(mqttTimeout, temp, 10);
  strcat(form, temp);
  strcat(form, "'><p>\n<label>Use MQTT:</label>"
    "<input type='checkbox' name='useMQTT'");
  if (useMQTT)
    strcat(form, " checked");
  strcat(form, "><p>\n<label>Reboot on MQTT fail:</label>"
    "<input type='checkbox' name='goAP'");
  if (onFail != 0)
    strcat(form, " checked");
  strcat(form, "><p>\n"
    "<input type='submit' class='shiny' name='SUBMIT' "
    "value='Submit MQTT configuration'>"
	  "</form>\n<p>\n");
  // Serial.print("Final length: ");
  // Serial.println(strlen(form));
  // Serial.println(form);
  page("MQTT configuration", form, 0);
} // end mqttConfig()

void mqttConfigRec() {
  if (deviceMode != 2)
    return;
  char apP[VARSIZE] = "", broker[VARSIZE] = "", port[VARSIZE] = "", 
    user[VARSIZE] = "", p1[VARSIZE] = "", p2[VARSIZE] = "", 
    itopic[VARSIZE] = "", otopic[VARSIZE] = "", 
    message[VARSIZE * 2 + 1] = "", timeout[VARSIZE] = "";
  boolean boxChecked = false, failReboot = false;
  if (server.hasArg("apPass"))
    strncpy(apP, server.arg("apPass").c_str(), VARSIZE - 1);
  if (server.hasArg("mqttBroker"))
    strncpy(broker, server.arg("mqttBroker").c_str(), VARSIZE - 1);
  Serial.print("Received broker: ");
  Serial.println(broker);
  if (server.hasArg("mqttPort"))
    strncpy(port, server.arg("mqttPort").c_str(), VARSIZE - 1);
  if (server.hasArg("mqttTimeout"))
    strncpy(timeout, server.arg("mqttTimeout").c_str(), VARSIZE - 1);
  if (server.hasArg("mqttUser"))
    strncpy(user, server.arg("mqttUser").c_str(), VARSIZE - 1);
  if (server.hasArg("mqttPass1"))
    strncpy(p1, server.arg("mqttPass1").c_str(), VARSIZE - 1);
  if (server.hasArg("mqttPass2"))
    strncpy(p2, server.arg("mqttPass2").c_str(), VARSIZE - 1);
  if (server.hasArg("inTopic"))
    strncpy(itopic, server.arg("inTopic").c_str(), VARSIZE - 1);
  if (server.hasArg("outTopic"))
    strncpy(otopic, server.arg("outTopic").c_str(), VARSIZE - 1);
  if (server.hasArg("useMQTT"))
    boxChecked = true;
  if (server.hasArg("goAP"))
    failReboot = true;

  if (strcmp(p1, p2) != 0)
    strcat(message, "New passwords do not match. ");
  if (strcmp(apPass, apP) != 0)
    strcat(message, "Invalid AP password. ");
  if (broker[0] == 0 && mqttBroker[0] == 0)
    strcat(message, "MQTT broker required, but not entered. ");
    
  if (strlen(message) > 0) {
    char temp[VARSIZE * 2 + 1] = "Ignoring all input. ";
    strcat(temp, message);
    strcpy(message, temp);
  }
  else {
    strcpy(message, "MQTT config input accepted. ");
    if (strlen(broker) > 0)
      strcpy(mqttBroker, broker);
    if (timeout[0] != 0)
      mqttTimeout = abs(atoi(timeout));
    if (mqttTimeout > 1024)
      mqttTimeout = 1024;
    if (port[0] != 0)
      mqttPort = abs(atoi(port));
    if (mqttPort < 1 || mqttPort > 65536)
      mqttPort = 1883;
    if (user[0] != 0)
      strcpy(mqttUser, user);
    if (strcmp(p1, p2) == 0)
      strcpy(mqttPass, p1);
    if (itopic[0] != 0)
      strcpy(inTopic, itopic);
    if (otopic[0] != 0)
      strcpy(outTopic, otopic);
    if (boxChecked)
      useMQTT = true;
    else
      useMQTT = false;
    if (failReboot)
      onFail = 1;
    else
      onFail = 0;
    saveMQTTConfig();
  }
  Serial.println(message);
  strcat(message, "<p>");
  page("Received MQTT configuration", message, 10);
} // end mqttConfigRec()

