void handleDelay(int howLong) {
  yield();
  if (howLong < 0)
    return;
  unsigned int startTime = millis();
  while (timeDiffNow(startTime) < howLong) {
    server.handleClient();
    if ((deviceMode == 1 || deviceMode == 2) && useMQTT)
      psClient.loop();
  }
} // end handleDelay()

void printStars(const char* input) {
  for (int i = 0; i < strlen(input); i++)
    Serial.print(F("*"));
} // end printStars()

char* generateHostName() {
  char lastThree[11] = "ESP-";
  byte temp, mac[6];
  WiFi.macAddress(mac);
  for (int i = 3; i < 6; i++) {
    temp = mac[i] / 16;
    if (temp > 9)
      lastThree[2 * i - 2] = temp + 55;
    else
      lastThree[2 * i - 2] = temp + 48;
    temp = mac[i] % 16;
    if (temp > 9)
      lastThree[2 * i - 1] = temp + 55;
    else
      lastThree[2 * i - 1] = temp + 48;
  }
  lastThree[10] = 0;
  Serial.print(F("Generated: "));
  Serial.print(lastThree);
  Serial.println(F(" from device MAC ID."));
  return lastThree;
} // end sixMAC()

long unsigned int timeDiffNow(long unsigned int a) {
  return timeDiff(a, millis());
} // end timeDiffNow

long unsigned int timeDiff(long unsigned int a, long unsigned int b) {
  if (a > b)
    return a - b;
  else
    return b - a;
} // end timeDiff()

void floatHelper(float input, char* output) {
  char floaters[3] = "";
  int timesHundred = input * 100;
  itoa(timesHundred / 100, output, 10);
  strcat(output, ".");
  itoa(timesHundred % 100, floaters, 10);
  strcat(output, floaters);
} // void floatHelper()

void slideFloat(float arr[], int arrSize) {
  for (int i = arrSize - 1; i > 0; i--)
    arr[i] = arr[i - 1];
} // end slideDown()

float averageFloat(float arr[], int arrSize) {
  float average = 0;
  int count = 0;
  for (int i = 0; i < arrSize; i++)
    if (arr[i] != -500) {
      average += arr[i];
      count++;
    }
  if (count == 0)
    return -500;
  return average / count;
} // end average()

void clearBuffer() {
  for (int i = 0; i < BUFFSIZE; i++)
    buffer[i] = 0;
} // end clearBuffer()

