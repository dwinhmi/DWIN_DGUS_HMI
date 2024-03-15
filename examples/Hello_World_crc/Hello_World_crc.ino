
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
 * Modified by Rtek1000 (Mar 07, 2024)
 * - Added CRC for functions, see source code to get more details
 * - Need CRC lib: https://github.com/RobTillaart/CRC
 * - Need to program the display using SD card with T5LCFG.CFG and 'CRC: ON'
 * - - Or send apropriated command to enable CRC (UART2 of HMI)
 * - CRC is error control, to improve communication reliability
 * - - Thanks: RobTillaart from GitHub
 *
 * Note: Check the Dwin display TX pin voltage (>3.3V?)
 *       before connecting to the ESP32/ESP8266 RX (<3.3V)
 */

#include <Arduino.h>
#include <DWIN.h>

#define ADDRESS_A "1010"
#define ADDRESS_B "1020"

#define DGUS_BAUD 115200
// 9600

#define PRINT_MSG 1

// If Using ESP 32
#if defined(ESP32)
#define DGUS_SERIAL Serial2
DWIN hmi(DGUS_SERIAL, 16, 17, DGUS_BAUD);

// If Using Arduino Uno
#else
DWIN hmi(2, 3, DGUS_BAUD);
#endif

uint32_t touch_sim_timer = 0;

// Event Occurs when response comes from HMI
void onHMIEvent(String address, int lastByte, String message, String response) {
  // Serial.println("Address : " + address + " | Data : " + String(lastByte, HEX) + " | Message : " + message + " | Response " + response);

  if (address == F("1002")) {
    // Take your custom action call
  }
}

void setup() {
  Serial.begin(115200);

  delay(1000);  // For ESP32 and Arduino IDE 2.0 Serial Monitor tab switch delay

  Serial.println(F("\n"));

  print_DWIN_HMI_log(F("Hello World - CRC"), "", true);

  hmi.echoEnabled(true);  // Enable Response Command Show: Yes

  /* waitGUIenabled(): (default is false: disabled)
    Enable Wait for the GUI to be free before sending the new Order (<300ms)
    If the display's TX serial port is not used, this function must be disabled (value: false)
    Some functions need waiting time (<20ms). (The reset needs <300ms)
  */
  hmi.waitGUIenabled(true);

  /* checkResponse(): (default is false: disabled)
    Enable response checking for sent command
    If the display's TX serial port is not used, this function must be disabled (value: false)
  */
  hmi.checkResponseEnabled(true);

  hmi.hmiCallBack(onHMIEvent);

  DWIN_HMI_set_brightness(100, 25, 1000);

  // init1();
}

void loop() {
  // Listen HMI Events
  //hmi.listen();

  if ((millis() - touch_sim_timer) > 5000) {
    touch_sim_timer = millis();

    DWIN_HMI_set_page(100);  // test Page

    DWIN_HMI_set_text(0x2000, "TEST");
  }
}

void init1() {
  // make 1 beep
  DWIN_HMI_beep();

  print_DWIN_HMI_log(F("Get version"), "", true);
  hmi.getHWVersion_crc();

  // make 1 beep
  DWIN_HMI_beep();

  DWIN_HMI_set_page(1);

  // make 1 beep
  DWIN_HMI_beep();

  DWIN_HMI_restart();

  // make 1 beep
  DWIN_HMI_beep();

  // Display an Icon (from 48.ICL file library) on VP Address of 'Basic Graphic' item
  DWIN_HMI_icon_display(0x1000, 48, 0, 0, 425);

  DWIN_HMI_set_rtc(24, 3, 6, 3, 12, 0, 0);

  DWIN_HMI_set_touchscreen(4, 100, 100);

  // make 1 beep
  DWIN_HMI_beep();

  int x_tp = random(95, 105);
  int y_tp = random(95, 105);
  DWIN_HMI_set_touchscreen(4, x_tp, y_tp);

  int page_rnd = random(0, 3);
  DWIN_HMI_set_page(page_rnd);

  DWIN_HMI_get_page();

  DWIN_HMI_set_brightness(15, 5, 1000);

  DWIN_HMI_get_brightness();
}

// There may also be a lack of encoding compatibility,
// and the characters will not be displayed correctly.
// - For example, when selecting the UNICODE encoding type,
// the microcontroller must send 2 bytes to write each character on the display.
// - If the program only uses basic (Western) characters 0x00~0x7F,
// try using GBK encoding, and you can only send 1 byte to write each character.
void DWIN_HMI_set_text(long address, String textData) {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.setText_crc(address, textData) == false) {
      print_DWIN_HMI_log(F("Set Text: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Set Text: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_set_brightness(uint8_t active_bright, uint8_t standby_bright, uint16_t sleep_timer) {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.setBrightness_crc(active_bright, standby_bright, sleep_timer) == false) {
      print_DWIN_HMI_log(F("Set Brightness: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Set Brightness: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_get_brightness() {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    int8_t resp = hmi.getBrightness_crc();

    if (resp < 0) {
      print_DWIN_HMI_log(F("Get Brightness: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("\nGet Brightness: "), String(resp), false);
      break;  // done
    }
  }
}

void DWIN_HMI_set_touchscreen(uint8_t mode, uint16_t x, uint16_t y) {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.setTouchscreen_crc(mode, x, y) == false) {
      print_DWIN_HMI_log(F("Set Touchscreen: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Set Touchscreen: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_set_rtc(uint8_t year, uint8_t month, uint8_t day, uint8_t week, uint8_t hour, uint8_t minute, uint8_t second) {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.setRTC_crc(year, month, day, week, hour, minute, second) == false) {
      print_DWIN_HMI_log(F("Set RTC: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Set RTC: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_set_page(uint8_t page_id) {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.setPage_crc(page_id) == false) {
      print_DWIN_HMI_log(F("Set Page: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Set Page: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_get_page() {
  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    int8_t resp = hmi.getPage_crc();

    if (resp < 0) {
      print_DWIN_HMI_log(F("Get Page: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Get Page: "), String(resp), false);
      break;  // done
    }
  }
}

void DWIN_HMI_beep() {
  return;  // mute

  for (uint8_t j = 0; j < 3; j++) {  // Try executing the command 3 times
    if (hmi.beepHMI_crc(0x05) == false) {
      print_DWIN_HMI_log(F("Beep: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Beep: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_icon_display(long addr, uint8_t icon_lib, int x, int y, int icon) {
  for (uint8_t i = 0; i < 3; i++) {  // Try executing the command 3 times
    // Display an Icon (from xx.ICL file library: icon_lib) on VP Address of 'Basic Graphic' item
    if (hmi.iconDisplay_crc(addr, icon_lib, x, y, icon) == false) {
      print_DWIN_HMI_log(F("Icon Display: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Icon Display: done"), "", true);
      break;  // done
    }
  }
}

void DWIN_HMI_restart() {
  // Make HMI to restart
  for (uint8_t i = 0; i < 3; i++) {           // Try executing the command 3 times
    if (hmi.restartHMI_crc(true) == false) {  // restartHMI_crc(true): wait to restart
      print_DWIN_HMI_log(F("Restart: error - bad response"), "", true);
    } else {
      print_DWIN_HMI_log(F("Restart: done"), "", true);
      break;  // done
    }
  }
}

void print_DWIN_HMI_log(String msg1, String msg2, bool send_ln) {
#if PRINT_MSG == 1
  Serial.print(F("\nDWIN HMI ~ "));
  Serial.print(msg1);
  Serial.print(msg2);

  if (send_ln) Serial.println();
#endif
  return;
}
