# DWIN DGUS HMI Arduino Library
Official Arduino Library for DWIN DGUS T5L HMI Display
Supporting Features till date.
- getHWVersion
- restartHMI()
- setPage()
- getPage()
- setBrightness()
- getBrightness()
- setVP()
- setText()
- beepHMI()
- listenEvents()

----

  Description and Additional Functions (send order without CRC):
  
  - echoEnabled(): Enable Response Command Show
  - echoSendEnabled(): Enable Send Command Show
  - waitGUIenabled(): Enable Wait for the GUI to be free before sending the new Order (<300ms)
  - checkResponseEnabled(): Enable response checking for sent command
  - listen(): Listen Touch Events & Messages from HMI
  - getHWVersion(): Get Version
  - restartHMI(): restart HMI
  - restartHMI_crc(): restart HMI (send order using CRC)
  - setPage(): set Particular Page
  - setPage_crc(): set Particular Page (send order using CRC)
  - getPage(): get Current Page ID
  - setBrightness(): set LCD Brightness
  - getBrightness(): set LCD Brightness
  - getBrightness_crc(): set LCD Brightness (send order using CRC)
  - setText(): set Data on VP Address
  - setDsBaudrate_9600(): Set DGUS_SERIAL UART baudrate 9600
  - setDsBaudrate_115200(): Set DGUS_SERIAL UART baudrate 115200
  - setVP(): set Byte on VP Address
  - beepHMI(): beep Buzzer for 1 sec
  - enableCRC(): set CRC on for UART2 (send order without CRC and wait-for-GUI-status-free)

  Description and Additional Functions (send order with CRC):
  - getHWVersion_crc(): Get Version (send order using CRC)
  - getGUIstatus_crc(): Get GUI-status (send order using CRC)
  - waitGUIstatusFree_crc(): Wait for GUI-status Free (send order using CRC)
  - disableCRC(): set CRC off for UART2 (send order using CRC)
  - setTouchscreen_crc(): set Touchscreen - TP operation simulation (send order using CRC)
  - setRTC_crc(): set Particular Page (send order using CRC)
  - getPage_crc(): get Current Page ID (send order using CRC)
  - setBrightness_crc(): set LCD Brightness and sleep timer (send order using CRC)
  - setText_crc(): set Data on VP Address (send order using CRC)
  - iconDisplay_crc(): Icon Display (from 48.ICL file library) on VP Address of 'Basic Graphic' item (send order using CRC)
  - setBaudrate_9600_crc(): Set UART baudrate 9600 (send order using CRC)
  - setBaudrate_115200_crc(): Set UART baudrate 115200 (send order using CRC)
  - setVP_crc(): set Byte on VP Address (send order using CRC)
  - setMultSeqVP_crc(): set Multiple and Sequential Words (16-bit) on VP Address (send order using CRC)
  - beepHMI_crc(): beep Buzzer for 1 sec or time value (0x00-0xFF) (send order using CRC)

----

## Usage
Download the Library and extract the folder in the libraries of Arduino IDE
#### Include DWIN Library (eg. DWIN.h) 
```C++
#include <DWIN.h>
```

#### Initialize the hmi Object with Rx | Tx Pins and Baud rate
```C++
// If Using ESP32 Or Arduino Mega 
#if defined(ESP32)
  #define DGUS_SERIAL Serial2
  DWIN hmi(DGUS_SERIAL, 16, 17, DGUS_BAUD); // 16 Rx Pin | 17 Tx Pin
// If Using Arduino Uno
#else
  DWIN hmi(2, 3, DGUS_BAUD);    // 2 Rx Pin | 3 Tx Pin
#endif
```

#### Define callback Function
```C++
// Event Occurs when response comes from HMI
void onHMIEvent(String address, int lastByte, String message, String response){  
  Serial.println("OnEvent : [ A : " + address + " | D : "+ String(lastByte, HEX)+ " | M : "+message+" | R : "+response+ " ]"); 
  if (address == "1002"){
  // Take your custom action call
  }
}
```

#### In void setup()
```C++
  hmi.echoEnabled(false);      // To get Response command from HMI
  hmi.hmiCallBack(onHMIEvent); // set callback Function
```

#### In loop()
```C++
  // Listen HMI Events
  hmi.listen();
```

---
