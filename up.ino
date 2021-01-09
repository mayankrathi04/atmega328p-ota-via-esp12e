//:00000001FF
void initFS() {
  SPIFFS.begin();
}
struct hexLine {
  uint8_t startChar[1];
  uint8_t lineDataSize[2];
  uint8_t address[4];
  uint8_t dType[2];
  uint8_t checksum[2];
} lineEncoded;
struct hexLineD {
  uint8_t lineDataSize;
  uint16_t address;
  uint8_t dType;
  uint8_t checksum;
} lineDecoded;
int programHardwareChip() {
  File codeFile = SPIFFS.open("hardwareCodeFile.hex", "r");
  if (!codeFile) {
    return -1;
  }
  int codeFileSize = codeFile.size();
  uint16_t currentPage = 0;
  uint8_t bytesUploaded = 0;
  for (int i = 0, ii = 1; i < codeFileSize; i++, ii++) {
    if (bytesUploaded % 128 == 0) {
      Serial.write(0x55);
      uint8_t lowAddr = (uint8_t)(currentPage & 0xFF);
      uint8_t highAddr = (uint8_t)((currentPage >> 8) & 0xFF);
      Serial.write(lowAddr);
      Serial.write(highAddr);
      Serial.write(0x20);
      int isAck = checkAck((char)0x14);
      if (isAck == 1) {
        delay(5);
        printDebugData("Programming address:" + String(currentPage) + "\r\n");
        delay(10);
        Serial.write(0x64);
        Serial.write(0);
        Serial.write(0x80);
        Serial.write(0x46);
      }
      else {
        return -1;
      }
    }
    codeFile.read(lineEncoded.startChar, 1);
    if (lineEncoded.startChar[0] == 0x3A) {
      printDebugData("Hex line " + String(ii) + "\r\n");
      codeFile.read(lineEncoded.lineDataSize, 2);
      decodeLineHex0();
      if (lineDecoded.lineDataSize == 0) {
        if (bytesUploaded<128) {
            for (int bb = bytesUploaded; bb < 128  ; bb = bb + 1) {
              Serial.write(0xFF);
              bytesUploaded++;
            }
            //bytesUploaded=bytesUploaded-1;
            printDebugData("Bytes uploaded" + String(bytesUploaded) + "\r\n");
            Serial.write(0x20);
            int isAck = checkAck((char)0x14);
            if (isAck == 1) {
              printDebugData("Programmed\r\n");
              return 1;
            }
          }
        return 1;
      }
      codeFile.read(lineEncoded.address, 4);
      codeFile.read(lineEncoded.dType, 2);
      decodeLineHex1();
      uint8_t codeData[lineDecoded.lineDataSize * 2];
      codeFile.read(codeData, (int)lineDecoded.lineDataSize * 2);
      codeFile.read(lineEncoded.checksum, 2);
      uint8_t codeDataD[lineDecoded.lineDataSize];
      uint8_t newLine[2];
      codeFile.read(newLine, 2);
      if (newLine[0] == '\r' && newLine[1] == '\n') {
        decodeLineHex2(codeData, codeDataD);
        //printDebugData("Size:" + String(lineDecoded.lineDataSize) + ",Checksum:" + String(lineDecoded.checksum) + "\r\n");
        bool isCorr = verifyCheckSum(codeDataD, codeDataD);
        if (isCorr) {
          for (int bb = 0; bb < lineDecoded.lineDataSize; bb = bb + 1) {
            Serial.write(codeDataD[bb]);
            bytesUploaded++;
          }
        }
      }
    }
    if (bytesUploaded == 128) {
      Serial.write(0x20);
      int isAck = checkAck((char)0x14);
      if (isAck == 1) {
        printDebugData("Programmed\r\n");
        delay(10);
      }
      else {
        return -1;
      }
      bytesUploaded = 0;
      currentPage = currentPage + 0x40;
    }
    i = i + 12 + (lineDecoded.lineDataSize * 2);
    //printDebugData("Code File Size:" + String(codeFileSize) + "\r\n");
    //printDebugData("Uploaded:" + String(i) + "\r\n\r\n");
  }
  return -1;
}
bool verifyCheckSum(uint8_t checks[], uint8_t codeD[]) {
  return true;
  /*
    int sum=0;
    sum=lineDecoded.lineDataSize+(lineDecoded.address>>8)+(lineDecoded.address&0xFF);
    for(int bd=0;bd<lineDecoded.lineDataSize;bd++){
    sum=sum+codeD[bd];
    }
    if(((~(sum&0xFF))+1)&0xFF==checks[0]){
    return true;
    }
    else{
    return false;
    }
  */
}
void decodeLineHex2(uint8_t codeE[], uint8_t codeD[]) {
  // printDebugData("-----" + String(lineEncoded.checksum[0]) + "----" + String(lineEncoded.checksum[1]) + "\r\n");
  lineDecoded.checksum = getByteFromHex2(lineEncoded.checksum[0], lineEncoded.checksum[1]);
  for (int j = 0, k = 0; j < (lineDecoded.lineDataSize * 2); j = j + 2, k++) {
    codeD[k] = getByteFromHex2(codeE[j], codeE[j + 1]);
  }
}
void decodeLineHex1() {
  lineDecoded.address = getByteFromHex4(lineEncoded.address[0], lineEncoded.address[1], lineEncoded.address[2], lineEncoded.address[3]);
  lineDecoded.dType = getByteFromHex2(lineEncoded.dType[0], lineEncoded.dType[1]);
}
void decodeLineHex0() {
  lineDecoded.lineDataSize = getByteFromHex2(lineEncoded.lineDataSize[0], lineEncoded.lineDataSize[1]);
}
uint8_t getByteFromHex2(uint8_t d1, uint8_t d2) {
  uint8_t res = 0;
  res = hexCharToInt(d1) << 4 | hexCharToInt(d2);
  return res;
}
uint16_t getByteFromHex4(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4) {
  uint16_t res = 0;
  res = hexCharToInt(d1);
  res = res << 4;
  res = res | hexCharToInt(d2);
  res = res << 4;
  res = res | hexCharToInt(d3);
  res = res << 4;
  res = res | hexCharToInt(d4);
  return res;
}
uint8_t hexCharToInt(char ch) {
  uint8_t returnType;
  switch (ch)
  {
    case '0':
      returnType = 0;
      break;
    case  '1' :
      returnType = 1;
      break;
    case  '2':
      returnType = 2;
      break;
    case  '3':
      returnType = 3;
      break;
    case  '4' :
      returnType = 4;
      break;
    case  '5':
      returnType = 5;
      break;
    case  '6':
      returnType = 6;
      break;
    case  '7':
      returnType = 7;
      break;
    case  '8':
      returnType = 8;
      break;
    case  '9':
      returnType = 9;
      break;
    case  'A':
      returnType = 10;
      break;
    case  'B':
      returnType = 11;
      break;
    case  'C':
      returnType = 12;
      break;
    case  'D':
      returnType = 13;
      break;
    case  'E':
      returnType = 14;
      break;
    case  'F' :
      returnType = 15;
      break;
    default:
      returnType = 0;
      break;
  }
  return returnType;
}
int readSignature() {
  long st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No sig 1\r\n");
      return -1;
    }
  }
  char resp1 = (char)Serial.read();
  st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No sig 2\r\n");
      return -1;
    }
  }
  char resp2 = (char)Serial.read();
  st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No sig 3\r\n");
      return -1;
    }
  }
  char resp3 = (char)Serial.read();
  printDebugData(String((int)resp1) + " " + String((int)resp2) + " " + String((int)resp3) + "\r\n");
  verifySpace();
  return 1;
}
int checkAckk(char c) {
  long st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No ack\r\n");
      return -1;
    }
  }
  char resp1 = (char)Serial.read();
  if (resp1 == c)
    return 1;
  else
    return -1;
}
int checkAck(char c) {
  long st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No ack\r\n");
      return -1;
    }
  }
  char resp1 = (char)Serial.read();
  // printDebugData("Got ack 1\r\n");
  if (verifySpace()) {
    return 1;
  }
  else {
    printDebugData("Wrong ack\r\n");
    return 2;
  }
}
bool verifySpace() {
  long st = millis();
  while (!Serial.available()) {
    if ((millis() - st) > 2000) {
      printDebugData("No ack\r\n");
      return false;
    }
  }
  char resp2 = (char)Serial.read();
  // printDebugData("Got ack 2\r\n");
  if (resp2 == 0x10) {
    return true;
  }
  else {
    printDebugData("Wrong ack\r\n");
    return false;
  }
}
bool sendCommand(uint8_t s) {
  boolean isExecuted = false;
  String actResponse = "";
  uint32_t st = millis();
  while (millis() - st < 3000) {
    while (Serial.available()) {
      actResponse += String(Serial.read());
    }
    if (actResponse == "2016") {
      //printDebugData("Command Executed Successfully\r\n");
      return true;
    }
    if (!isExecuted) {
      Serial.write(s);
      Serial.write(0x20);
      delay(500);
    }
  }
  return false;
}
bool sendCommand1() {
  boolean isExecuted = false;
  String actResponse;
  uint32_t st = millis();
  while (millis() - st < 4000) {
    while (Serial.available()) {
      actResponse += String(Serial.read());
    }
    if (actResponse == "2016") {
      //printDebugData("Command Executed Successfully\r\n");
      return true;
    }
    if (!isExecuted) {
      Serial.write("\x20");
      delay(70);
    }
  }
  return false;
}
void resetHardwareChip() {
  //RESET MODULE
  digitalWrite(HARDWARE_CHIP_RESET, LOW);
  delay(50);
  digitalWrite(HARDWARE_CHIP_RESET, HIGH);
  printDebugData("DEVICE RESET DONE\r\n");
  delay(10);
}
int exitProgrammingMode() {
  //LEAVE PROGRAMMING MODE
  sendCommand(0x51);
  printDebugData("DEVICE LEFT PROGRAMMING MODE\r\n");
  resetHardwareChip();
}
void updateHardwareChip() {
  int x1 = prepareChipForProgramming();
  if (x1 == 1) {
    int x2 = programHardwareChip();
    if (x2 == 1) {
      int x3 = exitProgrammingMode();
      printDebugData("Code Upload Successful\r\n");
    }
    else {
      printDebugData("Code Upload Unsuccessful..Error while uploading\r\n");
    }
  }
  else {
    printDebugData("Code Upload Unsuccessful..Error while setting device in programming mode\r\n");
  }
  uploadCodeInAtmega = false;
}
int prepareChipForProgramming() {
  resetHardwareChip();
  Serial.write("\x20");
  delay(20);
  printDebugData("HANDSHAKING...");
  if (sendCommand((uint8_t)0x30)) { //handshake
    printDebugData("DONE\r\n");
    sendCommand1();//just solve even odd issue
    printDebugData("DEVICE IN SYNC\r\n");
    printDebugData("SETTING DEVICE PARAMETERS...");
    if (setDeviceParameters()) {
      printDebugData("DONE\r\n");
      //setDeviceExtendedParamters();
      printDebugData("PUTTING DEVICE IN PROGRAMMING MODE...");
      if (sendCommand((uint8_t)0x50)) { //put in programming mode
        printDebugData("Done\r\n");
        return 1;
      }
      else {
        printDebugData("TIMEOUT..NOT DONE\r\n");
        return 0;
      }
    }
    else{
      printDebugData("TIMEOUT..NOT DONE\r\n");
    }
  }
  else {
    printDebugData("TIMEOUT..NOT DONE\r\n");
    return 0;
  }
}
bool setDeviceParameters() {
  Serial.write(0x42);//SET PROGRAMMING MODE PARAMTERS
  Serial.write(0x86);//DEVICE ID
  Serial.write(0);//REVISION ID 0
  Serial.write(0);//BOTH PARALLEL AND SERIAL MODE
  Serial.write(0x01);//FULL PARALLEL INTERFACE
  Serial.write(0x01);//POLLING CAN BE USED DURING SPI
  Serial.write(0x01);//PROGRAMMING INSTRUCTION SELF TIMED
  Serial.write(0x01);//1 LOCK BYTE
  Serial.write(0x03);//1 FUSE BYTE
  Serial.write(0xFF);//FLASH POLL VALUE 1
  Serial.write(0xFF);//FLASH POLL VALUE 2
  Serial.write(0xFF);//EEPROM POLL VALUE 1
  Serial.write(0xFF);//EEPROM POLL VALUE 2
  Serial.write(0);//PAGE SIZE HIGH BYTE
  Serial.write(0x80);//PAGE SIZE LOW BYTE
  Serial.write(0x04);//EEPROM SIZE HIGH BYTE
  Serial.write(0);//EEPROM SIZE LOW BYTE
  Serial.write(0);//FLASH SIZE BYTE 4(HIGH)
  Serial.write(0);//FLASH SIZE BYTE 3
  Serial.write(0x80);//FLASH SIZE BYTE 2
  Serial.write(0);//FLASH SIZE BYTE 1(LOW)
  Serial.write(0x20); //Sync_CRC_EOP
  String actResponse = "";
  uint32_t st = millis();
  while (millis() - st < 3000) {
    while (Serial.available()) {
      actResponse += String(Serial.read());
    }
    if (actResponse == "2016") {
      //printDebugData("Command Executed Successfully\r\n");
      return true;
    }
  }
  return false;
}
bool setDeviceExtendedParamters() {
  Serial.write(0x45); //SET EXTENDED FUSE BYTES
  Serial.write(0x05);
  Serial.write(0x04);
  Serial.write(0xD7);
  Serial.write(0xC2);
  Serial.write(0);
  Serial.write(0x20);//Sync_CRC_EOP
}
