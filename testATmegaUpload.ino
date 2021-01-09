/*
  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `\ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#define HARDWARE_CHIP_RESET D0
String hname="";
const char* ssid = "****";
const char* password = "*****";
File fsUploadFile;
bool uploadCodeInAtmega = false;
ESP8266WebServer server(80);
void setup(void) {
  WiFi.begin(ssid, password);
  hname="Node3";
  WiFi.hostname(hname);
  MDNS.begin(hname);
  Serial.begin(115200, SERIAL_8N1);
  SPIFFS.begin();
  pinMode(HARDWARE_CHIP_RESET,OUTPUT);
  digitalWrite(HARDWARE_CHIP_RESET, HIGH);
  startDebugOverAir();
  setupOta();
  server.onNotFound([]() {
    server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/", HTTP_POST, [](){server.send(200);printDebugData("Serving Page /upload\r\n");},handleFileUpload);
  server.on("/", HTTP_GET, []() {
    printDebugData("Serving Page /\r\n");
    server.send(200, "text/html","<!DOCTYPE html><html><head></head><body><form method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form></body></html>");
  });
  server.begin();
  printDebugData("HTTP server started");
}
void loop(void) {
  otaUtil();
  debuggingUtil();
  server.handleClient();
  MDNS.update();
  if (uploadCodeInAtmega) {
    clearRx();
    updateHardwareChip();
  }
}
void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    printDebugData("handleFileUpload Name: "+filename+"\r\n");
    fsUploadFile = SPIFFS.open("hardwareCodeFile.hex", "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      printDebugData("handleFileUpload Size: ");
      printDebugData(String(upload.totalSize)+"\r\n");
      server.sendHeader("Location", "/");     // Redirect the client to the success page
      server.send(303);
      uploadCodeInAtmega = true;
    }
    else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void clearRx() {
  while (Serial.available()) {
    char x = Serial.read();
  }
}
