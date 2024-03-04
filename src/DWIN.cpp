/*
* DWIN DGUS DWIN Library for Arduino Uno | ESP32 
* This Library Supports all Basic Function
* Created by Tejeet ( tejeet@dwin.com.cn ) 
* Please Checkout Latest Offerings FROM DWIN 
* Here : https://www.dwin-global.com/
*/

/*
 * Modified by Rtek1000 (Mar 04, 2024)
 * - Added CRC (when submitting the order *): (Modbus with swapped bytes)
 * - - (*): beepHMI_crc(time) [default time: 0x7D]
 * - Need CRC lib: https://github.com/RobTillaart/CRC
 * - Need to program the display using SD card with T5LCFG.CFG and 'CRC: ON'
 * - CRC is error control, to improve communication reliability
 */
 
#include "DWIN.h"
#include <stdio.h>

#include <CRC16.h>

CRC16 crc(CRC16_MODBUS_POLYNOME,
          CRC16_MODBUS_INITIAL,
          CRC16_MODBUS_XOR_OUT,
          CRC16_MODBUS_REV_IN,
          CRC16_MODBUS_REV_OUT);

#define CMD_HEAD1           0x5A
#define CMD_HEAD2           0xA5
#define CMD_WRITE           0x82
#define CMD_READ            0x83

#define MIN_ASCII           32
#define MAX_ASCII           255

#define CMD_READ_TIMEOUT    50
#define READ_TIMEOUT        100

#define MAXLENGTH 250

#if defined(ESP32)
    DWIN::DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud){
        port.begin(baud, SERIAL_8N1, receivePin, transmitPin);
        init((Stream *)&port, false);
    }

#elif defined(ESP8266)
    DWIN::DWIN(uint8_t receivePin, uint8_t transmitPin, long baud){
        localSWserial = new SoftwareSerial(receivePin, transmitPin);
        localSWserial->begin(baud);
        init((Stream *)localSWserial, true);
    }
    DWIN::DWIN(SoftwareSerial& port, long baud){
        port.begin(baud);
        init((Stream *)&port, true);
    }
    DWIN::DWIN(Stream& port, long baud){
        init(&port, true);
    }

#else
    DWIN::DWIN(uint8_t rx, uint8_t tx, long baud) {
        localSWserial = new SoftwareSerial(rx, tx);
        localSWserial->begin(baud);
        _baud = baud;
        init((Stream *)localSWserial, true);
    }

#endif

static bool calcCRC(uint8_t *array, bool only_check);

void DWIN::init(Stream* port, bool isSoft){
    this->_dwinSerial = port;
    this->_isSoft = isSoft;
}


void DWIN::echoEnabled(bool echoEnabled){
    _echo = echoEnabled;
}

// Get Hardware Firmware Version of DWIN HMI
double DWIN::getHWVersion(){  //  HEX(5A A5 04 83 00 0F 01)
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x0F, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer)); 
    delay(10);
    return readCMDLastByte();
}

// Restart DWIN HMI
void DWIN::restartHMI(){  // HEX(5A A5 07 82 00 04 55 aa 5a a5 )
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x04, 0x55, 0xaa, CMD_HEAD1, CMD_HEAD2};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer)); 
    delay(10);
    readDWIN();
}

// SET DWIN Brightness
void DWIN::setBrightness(byte brightness){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_WRITE, 0x00, 0x82, brightness };
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// SET DWIN Brightness
byte DWIN::getBrightness(){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00, 0x31, 0x01 };
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    return readCMDLastByte();
}

// Change Page 
void DWIN::setPage(byte page){
    //5A A5 07 82 00 84 5a 01 00 02
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07, CMD_WRITE, 0x00, 0x84, 0x5A, 0x01, 0x00, page};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Get Current Page ID
byte DWIN::getPage(){
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x04, CMD_READ, 0x00 , 0x14, 0x01};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer)); 
    return readCMDLastByte();
}

// Set Text on VP Address
void DWIN::setText(long address, String textData){

    int dataLen = textData.length();
    byte startCMD[] = {CMD_HEAD1, CMD_HEAD2, dataLen+3 , CMD_WRITE, 
    (address >> 8) & 0xFF, (address) & 0xFF};
    byte dataCMD[dataLen];
    textData.getBytes(dataCMD, dataLen+1);
    byte sendBuffer[6+dataLen];

    memcpy(sendBuffer, startCMD, sizeof(startCMD));
    memcpy(sendBuffer+6, dataCMD, sizeof(dataCMD));

    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// Set Data on VP Address
void DWIN::setVP(long address, byte data){
    // 0x5A, 0xA5, 0x05, 0x82, 0x40, 0x20, 0x00, state
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05 , CMD_WRITE, (address >> 8) & 0xFF, (address) & 0xFF, 0x00, data};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// beep Buzzer for 1 Sec
void DWIN::beepHMI(){
    // 0x5A, 0xA5, 0x05, 0x82, 0x00, 0xA0, 0x00, 0x7D
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x05 , CMD_WRITE, 0x00, 0xA0, 0x00, 0x7D};
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

// beep Buzzer for x Sec (send order using CRC)
void DWIN::beepHMI_crc(uint8_t time){
    // 0x5A, 0xA5, 0x07, 0x82, 0x00, 0xA0, 0x00, TIME, CRC0, CRC1
    byte sendBuffer[] = {CMD_HEAD1, CMD_HEAD2, 0x07 , CMD_WRITE, 0x00, 0xA0, 0x00, time, 0x00, 0x00};
    
    calcCRC(sendBuffer, false);
    
    _dwinSerial->write(sendBuffer, sizeof(sendBuffer));
    readDWIN();
}

static bool calcCRC(uint8_t *array, bool only_check) {
  //CRC16_restart();
  crc.restart();

  uint8_t length = array[2] + 1;  //   is used enough to spend a local variable on the length

  //  check if length is valid.
  if ((length < 4) || (length > MAXLENGTH)) {
    return false;
  }

  for (int i = 3; i < length; i++) {
    //CRC16_add(array[i]);
    crc.add(array[i]);
  }

  //uint16_t res = CRC16_calc();
  //uint8_t res0 = res >> 8;
  //uint8_t res1 = res;

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

// SET CallBack Event
void DWIN::hmiCallBack(hmiListener callBack){
    listenerCallback = callBack;
}

// Listen For incoming callback  event from HMI
void DWIN::listen(){
     handle();
}

String DWIN::readDWIN(){
      //* This has to only be enabled for Software serial
    #if defined(DWIN_SOFTSERIAL)
        if(_isSoft){
            ((SoftwareSerial *)_dwinSerial)->listen(); // Start software serial listen
        }  
    #endif

    String resp;
    unsigned long startTime = millis(); // Start time for Timeout

    while((millis() - startTime < READ_TIMEOUT)){
        if(_dwinSerial->available() > 0){
            int c = _dwinSerial->read();
            resp.concat(" "+String(c, HEX));
        }
    }
    if (_echo){
        Serial.println("->> "+resp);
    }
    return resp;
}

String DWIN::checkHex(byte currentNo){
    if (currentNo < 10){
        return "0"+String(currentNo, HEX);
    }
    return String(currentNo, HEX);
}

String DWIN::handle(){

    int lastByte;
    String response;
    String address;
    String message;
    bool isSubstr = false;
    bool messageEnd = true;
    bool isFirstByte = false;
    unsigned long startTime = millis(); 
  
    while((millis() - startTime < READ_TIMEOUT)){
        while(_dwinSerial->available() > 0){
            delay(10);
            int inhex = _dwinSerial->read();
            if (inhex == 90 || inhex == 165){
                isFirstByte = true;
                response.concat(checkHex(inhex)+" ");
                continue;
            }
            for(int i = 1 ; i <= inhex ;i++){
                int inByte = _dwinSerial->read();
                response.concat(checkHex(inByte)+" ");
                if (i <= 3){
                    if((i == 2) || (i == 3)){
                        address.concat(checkHex(inByte));
                    }
                    continue;
                }
                else{
                    if(messageEnd){
                        if (isSubstr && inByte != MAX_ASCII && inByte >= MIN_ASCII){
                            message += char(inByte);
                        }
                        else{
                            if(inByte == MAX_ASCII){
                                messageEnd = false;
                            }
                            isSubstr = true;
                        }
                    }
                }
                lastByte = inByte;
            }
        }
    }

    if (isFirstByte &&_echo){
        Serial.println("Address : " + address + " | Data : " + String(lastByte, HEX) + " | Message : " + message + " | Response " +response );
    }
    if (isFirstByte){
        listenerCallback(address, lastByte, message, response);
    }
    return response;
}



byte DWIN::readCMDLastByte(){
      //* This has to only be enabled for Software serial
    #if defined(DWIN_SOFTSERIAL)
        if(_isSoft){
            ((SoftwareSerial *)_dwinSerial)->listen(); // Start software serial listen
        }  
    #endif

    byte lastByte = -1;
    unsigned long startTime = millis(); // Start time for Timeout
    while((millis() - startTime < CMD_READ_TIMEOUT)){
        while(_dwinSerial->available() > 0){
            lastByte = _dwinSerial->read();
        }
    }
    return lastByte;
}


void DWIN::flushSerial(){
  Serial.flush();
  _dwinSerial->flush();
}
