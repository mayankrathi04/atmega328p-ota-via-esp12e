WiFiServer debugServer(23);
WiFiClient debugServerClient;
String dMode = "WIFI_DEBUG";
void startDebugOverAir() {
  debugServer.begin();
  debugServer.setNoDelay(true);
}
void debuggingUtil() {
  if (debugServer.hasClient()) {
    if (!debugServerClient || !debugServerClient.connected()) {
      if (debugServerClient) debugServerClient.stop();
      debugServerClient = debugServer.available();
    }
  }
}
void printDebugData(String x) {
  //Serial.print(x);
  if (debugServerClient && debugServerClient.connected()) {
    int a = x.length();
    uint8_t dat[a];
    for (int i = 0; i < a; i++) {
      dat[i] = byte(x.charAt(i));
    }
    debugServerClient.write(dat, a);
    delay(1);
  }
}
void ppp(char buf[]) {
 // Serial.print(buf);
  if (debugServerClient && debugServerClient.connected()) {
    debugServerClient.write(buf, 34);
    delay(1);
  }
}
void prints(char buf[])
{
  if (dMode == "WIFI_DEBUG") {
    if (debugServerClient && debugServerClient.connected()) {
      int a = strlen(buf);
      //Serial.print(buf);
      debugServerClient.write(buf, a);
      delay(1);
    }
  }
  else if (dMode == "SERIAL_DEBUG") {
    Serial.println(buf);
  }
}
