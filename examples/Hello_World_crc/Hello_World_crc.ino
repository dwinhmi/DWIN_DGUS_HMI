
/*
 * -----------------------------------------------------------------------------------------
 * DWIN Hello World Sketch   | Author : Tejeet ( tejdwin@gmail.com )
 * -----------------------------------------------------------------------------------------
 * This is DWIN HMI Library for Arduino Compatible Boards. More Info about Display Visit
 * Official Site --> https://www.dwin-global.com/
 * 
 * Example sketch/program showing how to initialize DWIN Hmi with Arduino Or ESP32 Boards
 * In this example we can see on setup loop we change the page no and set the brightness &
 * Listen to display Events from serial port
 * 
 * DWIN HMI to Various Boards Pinout Connection
 * -----------------------------------------------------------------------------------------
 * DWIN            ESP32         Arduino       ArduinoMega       ESP8266        
 * Pin             Pin           Pin           Pin               Pin  ( Coming Soon ) 
 * -----------------------------------------------------------------------------------------
 * 5V              Vin           5V            5V
 * GND             GND           GND           GND
 * RX2             16            2             18
 * TX2             17            3             19
 *------------------------------------------------------------------------------------------
 *
 * For More information Please Visit : https://github.com/dwinhmi/DWIN_DGUS_HMI
 *
 */

/*
 * Modified by Rtek1000 (Mar 04, 2024)
 * - Added CRC (when submitting the order *): (Modbus with swapped bytes)
 * - - (*): beepHMI_crc(time) [default time: 0x7D]
 * - Need CRC lib: https://github.com/RobTillaart/CRC
 * - Need to program the display using SD card with T5LCFG.CFG and 'CRC: ON'
 * - CRC is error control, to improve communication reliability
 * - - Thanks: RobTillaart from GitHub
 */

#include <Arduino.h>
#include <DWIN.h>

#define ADDRESS_A     "1010"
#define ADDRESS_B     "1020"

#define DGUS_BAUD     115200

// If Using ESP 32
#if defined(ESP32)
  #define DGUS_SERIAL Serial2
  DWIN hmi(DGUS_SERIAL, 16, 17, DGUS_BAUD);

// If Using Arduino Uno
#else
  DWIN hmi(2, 3, DGUS_BAUD);
#endif

// Event Occurs when response comes from HMI
void onHMIEvent(String address, int lastByte, String message, String response){  
  Serial.println("OnEvent : [ A : " + address + " | D : "+ String(lastByte, HEX)+ " | M : "+message+" | R : "+response+ " ]"); 
  if (address == "1002"){
  // Take your custom action call
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println("DWIN HMI ~ Hello World");
    hmi.echoEnabled(false);
    hmi.hmiCallBack(onHMIEvent);
    //hmi.setPage(1);

    Serial.println("DWIN HMI ~ beep");
    hmi.beepHMI_crc();
}

void loop() {
    // Listen HMI Events
    hmi.listen();
}
