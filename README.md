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

  ## Description and Additional Functions (send order without CRC):
  
  - Enable Response Command Show:
  - - echoEnabled()
  - Enable Send Command Show:
  - - echoSendEnabled()
  - Enable Wait for the GUI to be free before sending the new Order (<300ms):
  - - waitGUIenabled()
  - Enable response checking for sent command:
  - - checkResponseEnabled()
  - Listen Touch Events & Messages from HMI:
  - - listen()
  - Get Version:
  - - getHWVersion()
  - Restart HMI:
  - - restartHMI()
  - Set Particular Page:
  - - setPage()
  - Set Particular Page:
  - - setPage_crc()
  - Get Current Page ID:
  - - getPage()
  - Set LCD Brightness:
  - - setBrightness()
  - Set LCD Brightness:
  - - getBrightness()
  - Set LCD Brightness (send order using CRC)
  - - getBrightness_crc()
  - Set Data on VP Address:
  - - setText()
  - Set DGUS_SERIAL UART baudrate 9600:
  - - setDsBaudrate_9600()
  - Set DGUS_SERIAL UART baudrate 115200:
  - - setDsBaudrate_115200()
  - Set Byte on VP Address:
  - - setVP()
  - Beep Buzzer for 1 sec:
  - - beepHMI()
  - Set CRC on for UART2 (send order without CRC and wait-for-GUI-status-free)
  - - enableCRC():

  ## Description and Additional Functions (send order with CRC):
  - Restart HMI:
  - - restartHMI_crc()
  - Get Version:
  - - getHWVersion_crc()
  - Get GUI-status:
  - - getGUIstatus_crc()
  - Wait for GUI-status Free:
  - - waitGUIstatusFree_crc()
  - Set CRC off for UART2:
  - - disableCRC()
  - Set Touchscreen - TP operation simulation:
  - - setTouchscreen_crc()
  - Set Particular Page:
  - - setRTC_crc()
  - Get Current Page ID:
  - - getPage_crc()
  - Set LCD Brightness and sleep timer:
  - - setBrightness_crc()
  - Set Data on VP Address:
  - - setText_crc()
  - Icon Display (from *.ICL file library) on VP Address of 'Basic Graphic' item:
  - - iconDisplay_crc()
  - Set UART baudrate 9600:
  - - setBaudrate_9600_crc()
  - Set UART baudrate 115200:
  - - setBaudrate_115200_crc()
  - Set Byte on VP Address:
  - - setVP_crc()
  - Set Multiple and Sequential Words (16-bit) on VP address:
  - - setMultSeqVP_crc()
  - Beep Buzzer for 1 sec or time value (0x00-0xFF):
  - - beepHMI_crc():

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
