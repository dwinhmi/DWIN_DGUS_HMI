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


#ifndef DWIN_H
#define DWIN_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef ESP32
#include <SoftwareSerial.h>
#endif

#define DWIN_DEFAULT_BAUD_RATE 115200
#define ARDUINO_RX_PIN 10
#define ARDUINO_TX_PIN 11

class DWIN {

public:
// Using ESP32 Board
#if defined(ESP32)
  DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud = DWIN_DEFAULT_BAUD_RATE);
  DWIN(HardwareSerial* port, uint8_t receivePin, uint8_t transmitPin, long baud = DWIN_DEFAULT_BAUD_RATE)
    : DWIN(*port, receivePin, transmitPin, baud){};

// Using ESP8266 Board
#elif defined(ESP8266)
  DWIN(uint8_t receivePin, uint8_t transmitPin, long baud = DWIN_DEFAULT_BAUD_RATE);
  DWIN(SoftwareSerial& port, long baud = DWIN_DEFAULT_BAUD_RATE);
  DWIN(Stream& port, uint8_t long baud = DWIN_DEFAULT_BAUD_RATE);

// Using Arduino Board
#else
  DWIN(uint8_t rx = ARDUINO_RX_PIN, uint8_t tx = ARDUINO_TX_PIN, long baud = DWIN_DEFAULT_BAUD_RATE);
#endif

  // PUBLIC Methods

  // Enable Response Command Show
  void echoEnabled(bool enabled);
  
  // Enable Wait for the GUI to be free before sending the new Order (<300ms)
  void waitGUIenabled(bool waitEnabled);
  
  // Enable response checking for sent command
  void checkResponseEnabled(bool verifEnabled);

  // Listen Touch Events & Messages from HMI
  void listen();

  // Get Version
  double getHWVersion();
  // Get Version (send order using CRC)
  double getHWVersion_crc();
  
  // Get GUI-status (send order using CRC)
  bool getGUIstatus_crc();

  // Wait for GUI-status Free (send order using CRC)
  bool waitGUIstatusFree_crc(uint16_t timeout = 500);

  // restart HMI
  void restartHMI();
  // restart HMI (send order using CRC)
  bool restartHMI_crc(bool wait_to_restart = true);

  // set CRC on for UART2 (send order without CRC and wait-for-GUI-status-free)
  bool enableCRC();
  // set CRC off for UART2 (send order using CRC)
  bool disableCRC();

  // set Touchscreen - TP operation simulation (send order using CRC)
  bool setTouchscreen_crc(uint8_t mode, uint16_t pos_x, uint16_t pos_y);

  // set Particular Page (send order using CRC)
  bool setRTC_crc(byte year, byte month, byte day, byte week, byte hour, byte minute, byte second);
  
  // set Particular Page
  void setPage(byte pageID);
  // set Particular Page (send order using CRC)
  bool setPage_crc(byte pageID);

  // get Current Page ID
  byte getPage();
  // get Current Page ID (send order using CRC)
  int8_t getPage_crc();

  // set LCD Brightness
  void setBrightness(byte pConstrast);
  // set LCD Brightness (send order using CRC)
  bool setBrightness_crc(byte pConstrast_on = 0x64, byte pConstrast_off = 0x32, uint16_t sleep_timer = 1500);
  
  // set LCD Brightness
  byte getBrightness();
  // set LCD Brightness (send order using CRC)
  int8_t getBrightness_crc();

  // set Data on VP Address
  void setText(long address, String textData);
  // set Data on VP Address (send order using CRC)
  bool setText_crc(long address, String textData);

  // Icon Display (from 48.ICL file library) on VP Address of 'Basic Graphic' item (send order using CRC)
  bool iconDisplay_crc(long address, int x, int y, int icon);

  // Set DGUS_SERIAL UART baudrate 9600
  void setDsBaudrate_9600();
 
  // Set DGUS_SERIAL UART baudrate 115200
  void setDsBaudrate_115200();

  // Set UART baudrate 9600 (send order using CRC)
  bool setBaudrate_9600_crc();

  // Set UART baudrate 115200 (send order using CRC)
  bool setBaudrate_115200_crc();

  // set Byte on VP Address
  void setVP(long address, byte data);
  // set Byte on VP Address (send order using CRC)
  bool setVP_crc(long address, byte data);

  // beep Buzzer for 1 sec
  void beepHMI();
  // beep Buzzer for 1 sec or time value (0x00-0xFF) (send order using CRC)
  bool beepHMI_crc(uint8_t time = 0x7D);

  // Callback Function
  typedef void (*hmiListener)(String address, int lastByte, String message, String response);

  // CallBack Method
  void hmiCallBack(hmiListener callBackFunction);

private:

#ifndef ESP32
  SoftwareSerial* localSWserial = nullptr;
#endif

  Stream* _dwinSerial;          // DWIN Serial interface
  bool _isSoft = false;         // Is serial interface software
  long _baud = 0;               // DWIN HMI Baud rate
  bool _echo = false;           // Response Command Show
  bool _debug = false;          // Response Charactes Show
  bool _isConnected = false;    // Flag set on successful communication
  bool _wait = false;           // Flag Wait for the GUI to be free before sending the new Order (<300ms)
  bool _verify = false;         // Flag Enable response checking for sent command

  bool cbfunc_valid = false;
  hmiListener listenerCallback;

  void init(Stream* port, bool isSoft);
  void setDsBaudrate(uint32_t baud);
  byte readCMDLastByte();
  String readDWIN();
  bool readDWIN_array(byte *c_buffer, int16_t c_size);
  String handle();
  String checkHex(byte currentNo);
  void flushSerial();
  bool verifyResponse_4F4B();
  bool verifyResponse_4F4B_crc();
  bool calcCRC(uint8_t *array, uint16_t array_size, bool only_check);
};

#endif  // DWIN_H
