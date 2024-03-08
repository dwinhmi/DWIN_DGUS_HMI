/*
* DWIN DGUS DWIN Library for Arduino Uno | ESP32 
* This Library Supports all Basic Function
* Created by Tejeet ( tejeet@dwin.com.cn ) 
* Please Checkout Latest Offerings FROM DWIN 
* Here : https://www.dwin-global.com/
*/

/*
 * Modified by Rtek1000 (Mar 07, 2024)
 * - Added CRC for functions, see source code to get more details
 * - Need CRC lib: https://github.com/RobTillaart/CRC
 * - Need to program the display using SD card with T5LCFG.CFG and 'CRC: ON'
 * - - Or send apropriated command to enable CRC (UART2 of HMI)
 * - CRC is error control, to improve communication reliability
 * - - Thanks: RobTillaart from GitHub
 */

#include "DWIN.h"
#include <stdio.h>

#include <CRC16.h>

CRC16 crc(CRC16_MODBUS_POLYNOME,
          CRC16_MODBUS_INITIAL,
          CRC16_MODBUS_XOR_OUT,
          CRC16_MODBUS_REV_IN,
          CRC16_MODBUS_REV_OUT);

#define CMD_HEAD1 0x5A
#define CMD_HEAD2 0xA5
#define CMD_WRITE 0x82
#define CMD_READ 0x83

#define MIN_ASCII 32
#define MAX_ASCII 255

#define CMD_READ_TIMEOUT 50
#define READ_TIMEOUT 100
#define READ_TIMEOUT_20 20

#define MAXLENGTH 250

uint8_t DGUS_SERIAL_ID = 0;

#if defined(ESP32)
HardwareSerial* DGUS_port;

DWIN::DWIN(HardwareSerial &port, uint8_t receivePin, uint8_t transmitPin, long baud) {
  DGUS_port = &port;
  port.begin(baud, SERIAL_8N1, receivePin, transmitPin);
  init((Stream *)&port, false);
}

#elif defined(ESP8266)
SoftwareSerial* DGUS_port;

DWIN::DWIN(uint8_t receivePin, uint8_t transmitPin, long baud) {
  localSWserial = new SoftwareSerial(receivePin, transmitPin);
  DGUS_port = &localSWserial;
  
  localSWserial->begin(baud);
  init((Stream *)localSWserial, true);
}
DWIN::DWIN(SoftwareSerial &port, long baud) {
  DGUS_port = &port;
  
  port.begin(baud);
  init((Stream *)&port, true);
}
DWIN::DWIN(Stream &port, long baud) {
  DGUS_port = &port;
  
  init(&port, true);
}

#else
SoftwareSerial* DGUS_port;

DWIN::DWIN(uint8_t rx, uint8_t tx, long baud) {
  localSWserial = new SoftwareSerial(rx, tx);
  DGUS_port = &localSWserial;
  
  localSWserial->begin(baud);
  _baud = baud;
  init((Stream *)localSWserial, true);
}

#endif

void DWIN::init(Stream *port, bool isSoft) {
  this->_dwinSerial = port;
  this->_isSoft = isSoft;
}

// Enable Response Command Show
void DWIN::echoEnabled(bool echoEnabled) {
  _echo = echoEnabled;
}

// Enable Wait for the GUI to be free before sending the new Order (<300ms)
void DWIN::waitGUIenabled(bool waitEnabled) {
  _wait = waitEnabled;
}

// Enable response checking for sent command
void DWIN::checkResponseEnabled(bool verifEnabled) {
  _verify = verifEnabled;
}

// Get Hardware Firmware Version of DWIN HMI
double DWIN::getHWVersion() {  //  HEX(5A A5 04 83 00 0F 01)
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01 };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  delay(10);
  return readCMDLastByte();
}

// Get Hardware Firmware Version of DWIN HMI (send order using CRC)
double DWIN::getHWVersion_crc() {  //  HEX(5A A5 06 83 00 0F 01 xx xx)
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x06, CMD_READ, 0x00, 0x0F, 0x01, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  delay(10);
  readDWIN();
  return 0; //readCMDLastByte();
}

// Get GUI-status (send order using CRC)
bool DWIN::getGUIstatus_crc() {  // HEX(5A A5 06 83 0015 01 xx xx)
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x06, CMD_READ, 0x00, 0x15, 0x01, 0x00, 0x00 };
  
  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  //delay(10);
  
  // 5a a5 08 83 00 15 01 00 00 4a 24
  const byte respConstBuff[] = { CMD_HEAD1, CMD_HEAD2, 0x08, CMD_READ, 0x00, 0x15, 0x01, 0x00, 0x00, 0x4A, 0x24 };
  byte respBuffer[sizeof(respConstBuff)] = { 0 };
  
  if (readDWIN_array(respBuffer, sizeof(respBuffer)) == false) {
    return false;
  }
  
  if (respBuffer[8] == 1) { // 0x0000=free, 0x0001=processing     
    return false;
  }
    
  return true;
}

// Wait for GUI-status Free (send order using CRC)
bool DWIN::waitGUIstatusFree_crc(uint16_t timeout) {
  if (_wait) {
    unsigned long startTime = millis();  // Start time for Timeout

    while (((millis() - startTime) < timeout)) {
      if (getGUIstatus_crc()) {
        return true;
      }
    }
  
    if (_echo) {
      Serial.println("GUI-status: timeout");
    }
  }

  return false;
}

// Restart DWIN HMI
void DWIN::restartHMI() {  // HEX(5A A5 07 82 00 04 55 aa 5a a5 )
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2 };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  delay(10);
  readDWIN();
}

// Restart DWIN HMI (send order using CRC)
bool DWIN::restartHMI_crc(bool wait_to_restart) {  // HEX(5A A5 09 82 00 04 55 aa 5a a5 xx xx)
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x09, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2, 0x00, 0x00 };

  bool verif_resp = false;

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  
  //delay(10);
  if (_verify) {
    verif_resp = verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  } 

  if (wait_to_restart) {
      waitGUIstatusFree_crc(); 
  }
  
  return verif_resp;
}

// Set DWIN Brightness
void DWIN::setBrightness(byte brightness) {
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, 0x82, 0x00, brightness };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  readDWIN();
}

// Set DWIN Brightness (send order using CRC)
//
// --> To-do: sleep_timer is not working (DGUS kernel v65) but on T5LCFG.CFG it works
bool DWIN::setBrightness_crc(byte brightness_on, byte brightness_off, uint16_t sleep_timer) {
  // 5A A5 07 82 0082 HHLL TTTT cc-cc
  //
  // HH = (D3) Turn on brightness, 0x00-0x64; (Active state); (When backlight standby: the control is off)
  // LL = (D2) Turn off brightness, 0x00-0x64; (Standby state)
  // TT:TT = (D1:D0) open time /10 ms; Time to go from HH (D3) state to LL (D2) state
  //
  // For time of 10s: 10/0.01=1000; 1000=0x03E8; TT:TT=03:E8; Max. time: 0xFFFF=10 minutes and 55 seconds
  
  if (brightness_on > 0x64) brightness_on = 0x64;
  if (brightness_off > 0x64) brightness_off = 0x64;
  
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x82, brightness_on, brightness_off, 
                        (byte)((sleep_timer >> 8) & 0xFF), (byte)((sleep_timer) & 0xFF), 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

//  delay(10);

  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Get DWIN Brightness
byte DWIN::getBrightness() {
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x31, 0x01 };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  return readCMDLastByte();
}

// Get DWIN Brightness (send order using CRC)
int8_t DWIN::getBrightness_crc() {
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x06, CMD_READ, 0x00, 0x31, 0x01, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  
  // Reference: 5a a5 08 83 00 31 01 5a '05' ba 77
  byte respBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x08, CMD_READ, 0x00, 0x31, 0x01, 0x5A, 0x05, 0xBA, 0x77 };
  
  if (readDWIN_array(respBuffer, sizeof(respBuffer)) == true) {
    return respBuffer[8];
  }

  return -1; // error  
}

// Set CRC on for UART2 (send order without CRC and without wait-for-GUI-status-free)
bool DWIN::enableCRC() {
  // 5AA5 05 82 000C 5A80
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x00, 0x0C, 0x5A, 0x80 };

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  if (_verify) {
    return verifyResponse_4F4B();
  } else {
    readDWIN();
  }
  
  return false;
}

// Set CRC off for UART2 (send order using CRC)
bool DWIN::disableCRC() {
  // 5AA5 07 82 000C 5A00 A6BD
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x0C, 0x5A, 0x00, 0xA6, 0xBD };

  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  // delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Set Touchscreen - TP operation simulation (send order using CRC)
bool DWIN::setTouchscreen_crc(uint8_t mode, uint16_t pos_x, uint16_t pos_y) {
  // 5AA5 0B 82 00D4 5AA5 00mm xxxx yyyy cc-cc
  //
  // m=mode (1=press; 2=release; 3=continue pressing, 4=click)
  // xxxx=pos_x
  // yyyy=pos_y
  // cc= crc
  // cc= crc
  //
  // After simulating mode 0x0001 and 0x0003, must simulate 0x0002.
  
  if (mode < 1) mode = 1;
  if (mode > 4) mode = 4;

  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x0D, CMD_WRITE, 0x00, 0xD4,
                        0x5A, 0xA5, 0x00, mode,
                        (byte)((pos_x >> 8) & 0xFF), (byte)((pos_x) & 0xFF),
                        (byte)((pos_y >> 8) & 0xFF), (byte)((pos_y) & 0xFF),
                        0x00, 0x00 };
  
  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();
  
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  // delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Set RTC (send order using CRC)
bool DWIN::setRTC_crc(byte year, byte month, byte day, byte week, byte hour, byte minute, byte second) {
  // hh-hh 0D 82 0010 D7 D6 D5 D4 D3 D2 D1 D0 cc-cc
  //
  // hh=CMD_HEAD1
  // hh=CMD_HEAD2
  // D7=Year   (0-0x63)
  // D6=Month  (0-0x0C)
  // D5=Day    (0-0x1F)
  // D4=Week   (0-0x6)
  // D3=Hour   (0-0x17)
  // D2=Minute (0-0x3B)
  // D1=Second (0-0x3B)
  // cc= crc
  // cc= crc
  
  if (year > 0x63) year = 0x63;
  if (month > 0x0C) month = 0x0C;
  if (day > 0x1F) day = 0x1F;
  if (week > 0x6) week = 0x6;
  if (hour > 0x17) hour = 0x17;
  if (minute > 0x3B) minute = 0x3B;
  if (second > 0x3B) second = 0x3B;

  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x0D, CMD_WRITE, 0x00, 0x10,
                        year, month, day, week,
                        hour, minute, second, 0x00, 0x00, 0x00 };
  
  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();
  
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
   delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Change Page
void DWIN::setPage(byte pageID) {
  //5A A5 07 82 00 84 5a 01 00 02
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, pageID };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  readDWIN();
}

// Change Page (send order using CRC)
bool DWIN::setPage_crc(byte pageID) {
  //5A A5 09 82 00 84 5a 01 00 02 XX XX
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x09, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, pageID, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  //  delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Get Current Page ID
byte DWIN::getPage() {
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x14, 0x01 };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  return readCMDLastByte();
}

// Get Current Page ID (send order using CRC)
int8_t DWIN::getPage_crc() {
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x06, CMD_READ, 0x00, 0x14, 0x01, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  // Reference: 5a a5 08 83 00 14 01 00 '00' 4b d8
  byte respBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x08, CMD_READ, 0x00, 0x14, 0x01, 0x00, 0x00, 0x4B, 0xD8 };
  
  if (readDWIN_array(respBuffer, sizeof(respBuffer)) == true) {
    return respBuffer[8];
  }
  
  return -1; // error
}

// Set Text on VP Address
void DWIN::setText(long address, String textData) {

  int dataLen = textData.length();
  byte startCMD[] = { CMD_HEAD1, CMD_HEAD2, (byte)(dataLen + 3), CMD_WRITE,
                      (byte)((address >> 8) & 0xFF), (byte)((address) & 0xFF) };
  byte dataCMD[dataLen];
  textData.getBytes(dataCMD, dataLen + 1);
  byte sendBuffer[6 + dataLen];

  memcpy(sendBuffer, startCMD, sizeof(startCMD));
  memcpy(sendBuffer + 6, dataCMD, sizeof(dataCMD));

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  readDWIN();
}

// Set Text on VP Address (send order using CRC)
bool DWIN::setText_crc(long address, String textData) {
  // Reference: 5aa5 06 83 20 00 30 cc-cc

  int dataLen = textData.length();
  
  byte startCMD[] = { CMD_HEAD1, CMD_HEAD2, (byte)(dataLen + 5), CMD_WRITE,
                      (byte)((address >> 8) & 0xFF), (byte)((address) & 0xFF) };
  byte dataCMD[dataLen + 2]; // Include CRC

  textData.getBytes(dataCMD, dataLen + 1);
  byte sendBuffer[sizeof(startCMD) + sizeof(dataCMD)];
  
  memcpy(sendBuffer, startCMD, sizeof(startCMD));
  memcpy(sendBuffer + 6, dataCMD, sizeof(dataCMD));

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Icon Display (from 48.ICL file library) on VP Address of 'Basic Graphic' item (send order using CRC)
bool DWIN::iconDisplay_crc(long address, int x, int y, int i) {
  //hh-hh 11 82 aa-aa 30-07 00-01 xx-xx yy-yy ii-ii FF-00 cc-cc
  //
  //hh-hh: CMD_HEAD1, CMD_HEAD2
  //aa-aa: address (VP)
  //xx-xx: x (x position)
  //yy-yy: y (y position)
  //ii-ii: i (icon ID)
  //cc-cc: CRC (automatic calculation, leave 0x00 to reserve space)
  
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x11, CMD_WRITE,
                        (byte)((address >> 8) & 0xFF), (byte)((address)&0xFF),
                        0x30, 0x07, 0x00, 0x01,
                        (byte)((x >> 8) & 0xFF), (byte)((x)&0xFF),
                        (byte)((y >> 8) & 0xFF), (byte)((y)&0xFF),
                        (byte)((i >> 8) & 0xFF), (byte)((i)&0xFF),
                        0xFF, 0x00, 0x00, 0x00 };
                        
  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  
//  delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// Set DGUS_SERIAL UART baudrate 9600
void DWIN::setDsBaudrate_9600(){
  setDsBaudrate(9600);
}

// Set DGUS_SERIAL UART baudrate 115200
void DWIN::setDsBaudrate_115200(){
  setDsBaudrate(115200);
}

// Set DGUS_SERIAL UART baudrate
void DWIN::setDsBaudrate(uint32_t baud){
  DGUS_port->begin(baud);
}

// Set UART baudrate 9600 (send order using CRC)
bool DWIN::setBaudrate_9600_crc() {
  // 5AA5 09 82 000C 5A00 0150 cc-cc
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x09, CMD_WRITE, 0x00, 0x0C, 0x5A, 0x00, 0x01, 0x50, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  //  delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }

  return false;
}

// Set UART baudrate 115200 (send order using CRC)
bool DWIN::setBaudrate_115200_crc() {
  // 5AA5 09 82 000C 5A00 001C cc-cc
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x09, CMD_WRITE, 0x00, 0x0C, 0x5A, 0x00, 0x00, 0x1C, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  //  delay(10);
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }

  return false;
}

// Set Data on VP Address
void DWIN::setVP(long address, byte data) {
  // 0x5A, 0xA5, 0x05, 0x82, 0x40, 0x20, 0x00, state
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE,
                        (byte)((address >> 8) & 0xFF), (byte)((address) & 0xFF),
                        0x00, data };
                        
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  readDWIN();
}

// Set Data on VP Address (send order using CRC)
bool DWIN::setVP_crc(long address, byte data) {
  // 0x5A, 0xA5, 0x07, 0x82, 0x40, 0x20, 0x00, state, 0x00, 0x00
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE,
                        (byte)((address >> 8) & 0xFF), (byte)((address) & 0xFF),
                        0x00, data, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));

  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }

  return false;
}

// Beep Buzzer for 1 Sec
void DWIN::beepHMI() {
  // 0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x00, 0xA0, 0x00, 0x7D };
  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  readDWIN();
}

// Beep Buzzer for x Sec (send order using CRC)
bool DWIN::beepHMI_crc(uint8_t time) {
  // 0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, TIME, CRC0, CRC1
  byte sendBuffer[] = { CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0xA0, 0x00, time, 0x00, 0x00 };

  calcCRC(sendBuffer, sizeof(sendBuffer), false);
  
  waitGUIstatusFree_crc();

  _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
  
  if (_verify) {
    return verifyResponse_4F4B_crc();
  } else {
    readDWIN();
  }
  
  return false;
}

// SET CallBack Event
void DWIN::hmiCallBack(hmiListener callBack) {
  listenerCallback = callBack;
}

// Listen For incoming callback  event from HMI
void DWIN::listen() {
  handle();
}

// Read response from HMI, return string
String DWIN::readDWIN() {
  //* This has to only be enabled for Software serial
#if defined(DWIN_SOFTSERIAL)
  if (_isSoft) {
    ((SoftwareSerial *)_dwinSerial)->listen();  // Start software serial listen
  }
#endif

  String resp = "";
  unsigned long startTime = millis();  // Start time for Timeout

  while (((millis() - startTime) < READ_TIMEOUT)) {
    if (_dwinSerial->available() > 0) {
      int c = _dwinSerial->read();
      
      if (c <= 9) {
        resp.concat(" 0" + String(c, HEX));
      } else {
        resp.concat(" " + String(c, HEX));
      }      
    }
  }
  if (_echo) {
    Serial.println("->> " + resp);
  }
  return resp;
}

// Read response from HMI, return bool and data via buffer pointer
bool DWIN::readDWIN_array(byte *c_buffer, int16_t c_size) {
  //* This has to only be enabled for Software serial
#if defined(DWIN_SOFTSERIAL)
  if (_isSoft) {
    ((SoftwareSerial *)_dwinSerial)->listen();  // Start software serial listen
  }
#endif

  int16_t index = 0;
  bool is_not_timeout_flag = false;
 
  unsigned long startTime = millis();  // Start time for Timeout

  while (((millis() - startTime) < READ_TIMEOUT_20)) {
    if (_dwinSerial->available() > 0) {
      byte c = _dwinSerial->read();
        
      if (index < c_size) {
        c_buffer[index] = c;

        index++;
      } else {        
        if (_echo) {
          Serial.println(F("\nreadDWIN_array() Error: Buffer overflow"));
        }
      }
      
      if (index == c_size) {
        is_not_timeout_flag = true;
        
        break;
      }
    }
  }
  
  if (calcCRC(c_buffer, c_size, true) == false) { // crc check
    return false;
  }
  
  if (is_not_timeout_flag == true) {
    return true;
  }
  
  return false;
}

String DWIN::checkHex(byte currentNo) {
  if (currentNo < 10) {
    return "0" + String(currentNo, HEX);
  }
  return String(currentNo, HEX);
}

String DWIN::handle() {
  int lastByte;
  String response;
  String address;
  String message;
  bool isSubstr = false;
  bool messageEnd = true;
  bool isFirstByte = false;
  unsigned long startTime = millis();

  while (((millis() - startTime) < READ_TIMEOUT)) {
    while (_dwinSerial->available() > 0) {
      delay(10);
      int inhex = _dwinSerial->read();
      
      if (inhex == 90 || inhex == 165) {
        isFirstByte = true;
        response.concat(checkHex(inhex) + " ");
        continue;
      }
      for (int i = 1; i <= inhex; i++) {
        int inByte = _dwinSerial->read();
        response.concat(checkHex(inByte) + " ");
        if (i <= 3) {
          if ((i == 2) || (i == 3)) {
            address.concat(checkHex(inByte));
          }
          continue;
        } else {
          if (messageEnd) {
            if (isSubstr && inByte != MAX_ASCII && inByte >= MIN_ASCII) {
              message += char(inByte);
            } else {
              if (inByte == MAX_ASCII) {
                messageEnd = false;
              }
              isSubstr = true;
            }
          }
        }
        lastByte = inByte;
      }
      
      inhex = 0;
    }
  }

  if (isFirstByte && _echo) {
    Serial.println("Address : " + address + " | Data : " + String(lastByte, HEX) +
                   " | Message : " + message + " | Response " + response);
  }
  
  if (isFirstByte) {
    listenerCallback(address, lastByte, message, response);
  }
  return response;
}

byte DWIN::readCMDLastByte() {
  //* This has to only be enabled for Software serial
#if defined(DWIN_SOFTSERIAL)
  if (_isSoft) {
    ((SoftwareSerial *)_dwinSerial)->listen();  // Start software serial listen
  }
#endif

  byte lastByte = -1;
  unsigned long startTime = millis();  // Start time for Timeout
  while (((millis() - startTime) < CMD_READ_TIMEOUT)) {
    while (_dwinSerial->available() > 0) {
      lastByte = _dwinSerial->read();
    }
  }
  return lastByte;
}

void DWIN::flushSerial() {
  Serial.flush();
  _dwinSerial->flush();
}

bool DWIN::verifyResponse_4F4B() {
  // Response from HMI (CRC disabled)
  // 5a a5 03 82 4f 4b
  
  const byte respConstBuff[] = { CMD_HEAD1, CMD_HEAD2, 0x03, CMD_WRITE, 0x4F, 0x4B };
  byte respBuffer[sizeof(respConstBuff)] = { 0 };

  readDWIN_array(respBuffer, sizeof(respBuffer));
    
  for (uint8_t i = 0; i < 6; i++) {
    if (respConstBuff[i] != respBuffer[i]) {     
      if (_echo) {
        Serial.print(F("\nverifyResponse_4F4B() Error ("));
        Serial.print(i, DEC);
        Serial.println(F(")"));
      }

      return false;
    }
  }
  
  return true;
}

bool DWIN::verifyResponse_4F4B_crc() {
  // Response from HMI
  // 5a a5 05 82 4f 4b a5 ef
  const byte respConstBuff[] = { CMD_HEAD1, CMD_HEAD2, 0x05, CMD_WRITE, 0x4F, 0x4B, 0xA5, 0xEF };
  byte respBuffer[sizeof(respConstBuff)] = { 0 };

  if (readDWIN_array(respBuffer, sizeof(respBuffer)) == false) {
    return false;
  }
    
  if ((respConstBuff[4] != respBuffer[4]) || (respConstBuff[5] != respBuffer[5])) {     
    if (_echo) {
      Serial.println(F("\nverifyResponse_4F4B_crc() Error"));
    }

    return false;
  }

  return true;
}

bool DWIN::calcCRC(uint8_t *array, uint16_t array_size, bool only_check) {
  //CRC16_restart();
  crc.restart();

  uint8_t length = array[2] + 1;  // is used enough to spend a local variable on the length

  // some protections for bad coding
  if ((length < 4) || (length > MAXLENGTH) || (length > (array_size - 2))) {
    if (_echo) {
      Serial.println(F("\ncalcCRC() Error: incorrect length"));
    }
        
    return false;
  }

  for (int i = 3; i < length; i++) {
    crc.add(array[i]);
  }

  uint16_t res = crc.calc();
  uint8_t res0 = res;
  uint8_t res1 = res >> 8;

  if ((array[length] == res0) && (array[length + 1] == res1)) {
    return true;
  }

  if (only_check == false) {
    array[length] = res0;
    array[length + 1] = res1;
  }

  return false;
}
