/*
* DWIN DGUS DWIN Library for Arduino Uno | ESP32 
* This Library Supports all Basic Function
* Created by Tejeet ( tejeet@dwin.com.cn ) 
* Please Checkout Latest Offerings FROM DWIN 
* Here : https://www.dwin-global.com/
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


#define DWIN_DEFAULT_BAUD_RATE      115200
#define ARDUINO_RX_PIN              10
#define ARDUINO_TX_PIN              11


class DWIN{

public:
    // Using ESP32 Board
    #if defined(ESP32)
    DWIN(HardwareSerial& port, uint8_t receivePin, uint8_t transmitPin, long baud=DWIN_DEFAULT_BAUD_RATE);
    DWIN(HardwareSerial* port, uint8_t receivePin, uint8_t transmitPin, long baud=DWIN_DEFAULT_BAUD_RATE) : DWIN(*port, receivePin, transmitPin, baud) {};
    
    // Using ESP8266 Board
    #elif defined(ESP8266)
    DWIN(uint8_t receivePin, uint8_t transmitPin, long baud=DWIN_DEFAULT_BAUD_RATE);
    DWIN(SoftwareSerial& port, long baud=DWIN_DEFAULT_BAUD_RATE);
    DWIN(Stream& port, uint8_t long baud=DWIN_DEFAULT_BAUD_RATE);

    // Using Arduino Board
    #else
    DWIN(uint8_t rx=ARDUINO_RX_PIN, uint8_t tx=ARDUINO_TX_PIN, long baud=DWIN_DEFAULT_BAUD_RATE);
    #endif


    // PUBLIC Methods

    void echoEnabled(bool enabled);
    // Listen Touch Events & Messages from HMI
    void listen();
    // Get Version
    double getHWVersion();
    // restart HMI
    void restartHMI();
    // set Particular Page
    void setPage(byte pageID);
    // get Current Page ID
    byte getPage();
    // set LCD Brightness
    void setBrightness(byte pConstrast);
    // set LCD Brightness
    byte getBrightness();
    // set Data on VP Address
    void setText(long address, String textData);
    // set Byte on VP Address
    void setVP(long address, byte data);
    // beep Buzzer for 1 sec
    void beepHMI();
    // Callback Function
    typedef void (*hmiListener) (String address, int lastByte, String message, String response);

    // CallBack Method
    void hmiCallBack(hmiListener callBackFunction);


private:

    #ifndef ESP32
    SoftwareSerial* localSWserial = nullptr; 
    #endif

    Stream* _dwinSerial;   // DWIN Serial interface
    bool _isSoft;          // Is serial interface software
    long _baud;              // DWIN HMI Baud rate
    bool _echo;            // Response Command Show
    bool _isConnected;     // Flag set on successful communication

    bool cbfunc_valid;
    hmiListener listenerCallback;

    void init(Stream* port, bool isSoft); 
    byte readCMDLastByte();
    String readDWIN();
    String handle();
    String checkHex(byte currentNo);
    void flushSerial();

};



#endif // DWIN_H