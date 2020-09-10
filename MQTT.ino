boolean callback(char* subTopic, byte* payload, unsigned int length) {
  if (!usingMQTT())
    return false;
  clearBuffer();
  if (length > 2990)
    length = 2990;
  strncpy(buffer, (char*) payload, length);
  Serial.print("Received message: ");
  Serial.println(buffer);
  if (strcmp(buffer, "Mode1") == 0 && deviceMode != 1)
    switchMode(1);
  else if (strcmp(buffer, "Mode2") == 0)
    switchMode(2);
  else if (strcmp(buffer, "Mode9") == 0)
    switchMode(9);
  else if (strcmp(buffer, "SuspendMQTT") == 0) {
    suspendMQTT();
  }
  else if (strcmp(buffer, "Reboot") == 0)
    reboot();
  else if (strcmp(buffer, "WipeSpiffs") == 0)
    wipeSpiffs();
  else if (strcmp(buffer, "Timeout900") == 0)
    timeout(900);
  else if (strcmp(buffer, "Timeout0") == 0)
    timeout(0);
  else
    specialMQTT(buffer);
  return true;
} // end callback()

boolean suspendMQTT() {
  Serial.println("Disabling MQTT communications.");
  useMQTT = false;
} // end suspendMQTT()

void timeout(int timeout) {
  mqttTimeout = abs(timeout);
  saveMQTTConfig();
} // end timeout()

boolean usingMQTT() {
  if (!useMQTT || deviceMode != 1)
    return false;
  return true;
} // end usingMQTT()

boolean sendMQTT(char* message) {
  if (!usingMQTT())
    return false;
  Serial.print("Sending MQTT message: ");
  Serial.println(message);

  if (!psClient.connected())
    reconnect();
  psClient.loop();
  return psClient.publish(outTopic, message, strlen(message) + 1);
} // end sendMQTT()

boolean mqttSetup() {
  if (!usingMQTT())
    return false;
  // Connect to MQTT broker and send welcome message. 
  psClient.setServer(mqttBroker, mqttPort);
  psClient.setCallback(callback);
  if (!psClient.connected())
    reconnect();
  psClient.loop();
  return true;
} // end mqttSetup()


boolean reconnect() {
  if (!usingMQTT())
    return true;
  static int failures = 0;
  long unsigned int startTime = millis();
  // Loop until reconnected or mqttTimeout surpassed.
  while (!psClient.connected() && usingMQTT()) {
    Serial.print("Attempting MQTT connection... ");
    if (psClient.connect(deviceName, mqttUser, mqttPass)) {
      Serial.print("Connected to MQTT Broker: ");
      Serial.println(mqttBroker);
      strcpy(buffer, deviceName);
      strcat(buffer, " connected.");
      psClient.publish(outTopic, buffer);
      psClient.subscribe(inTopic);
      failures = 0;
      return true;
    }
    else {
      failures++;
      Serial.print("Attempt failed, rc = ");
      Serial.print(psClient.state());
      Serial.println(", trying again in five seconds.");
      handleDelay(5 * 1000);
      commfail(failures);
    }
  }
  return false;
} // end reconnect()

