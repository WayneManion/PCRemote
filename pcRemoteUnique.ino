void initializePins() {
  pinMode(SWITCH, OUTPUT);
  digitalWrite(SWITCH, HIGH);
  pinMode(SENSE, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
} // end initializePins()

void setPins(int howLong) {
  Serial.print("Pressing switch for ");
  Serial.print(howLong / 999.999);
  Serial.println(" seconds.");
  digitalWrite(SWITCH, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  handleDelay(howLong);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(SWITCH, HIGH);
} // end setPins()

void setUniquePages() {
  server.on("/standardPress", standardPress);
  server.on("/longPress", longPress);
  server.on("/ultraPress", ultraPress);
} // end setUniquePages()

void specialHome() {
  char title[VARSIZE] = "Attached PC is ", body[513];
  strcpy(body, "<h1>PC attached to ");
  strcat(body, deviceName);
  strcat(body, " is powered ");
  if (digitalRead(SENSE)) {
    strcat(body, "off");
    strcat(title, "Off");
  }
  else {
    strcat(body, "on");
    strcat(title, "On");
  }
  strcat(body, "</h1>");
  if (deviceMode == 2 || deviceMode == 9) {
    strcat(body, 
      "\n<a href='/standardPress'><button class='shiny'>Momentary Press</button></a><p>\n"
      "<a href='/longPress'><button class='shiny'>Press for five seconds</button></a><p>\n"
      "<a href='/ultraPress'><button class='shiny'>Press for thirty seconds</button></a><p>\n");
  }

  page(title, body, 0);
} // end specialHome()

void standardPress() {
  genericPress(200, "Short Press", "<h1>Pressing power button for a moment.</h1>");
} // end standardPress()

void longPress() {
  genericPress(5000, "Long Press", "<h1>Pressing power button for five seconds.</h1>");
} // end longPress()

void ultraPress() {
  genericPress(30000, "Ultra Press", "<h1>Pressing power button for thirty seconds.</h1>");
} // end longPress()

void genericPress (int howLong, char* title, char* body) {
  if (deviceMode > 1) {
    page(title, body, 1);
    setPins(howLong);
    return;
  }
} // end genericPress()

void specialMQTT(const char* inMessage) {
  char deviceState[VARSIZE] = "Attached device is ";
  if (digitalRead(SENSE))
    strcat(deviceState, "off.");
  else
    strcat(deviceState, "on.");
  if (strcmp(inMessage, "ShortPress") == 0)
    setPins(200);
  else if (strcmp(inMessage, "LongPress") == 0)
    setPins(5000);
  else if (strcmp(inMessage, "UltraPress") == 0)
    setPins(30000);
  else if (strcmp(inMessage, "ReportState") == 0)
    sendMQTT(deviceState);
} // end specialMQTT()

int commfail(int failTimes) {
  if (mqttTimeout == 0)
    return -1;
  if (failTimes < mqttTimeout)
    return (mqttTimeout - failTimes);
  if (onFail == 0) 
    suspendMQTT();
  else if (onFail == 1)
    switchMode(9);
  return -1;
} // end commfail()

