#include <WiFiUdp.h>
#include <ArduinoOTA.h>
void setupOta() {
  ArduinoOTA.setHostname(hname.c_str());
  ArduinoOTA.setPassword("qwerty123.");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
   });
  ArduinoOTA.onEnd([]() {
    printDebugData("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
   // Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      printDebugData("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      printDebugData("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      printDebugData("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      printDebugData("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      printDebugData("End Failed");
    }
  });
  ArduinoOTA.begin();
}
void otaUtil() {
  ArduinoOTA.handle();
}
