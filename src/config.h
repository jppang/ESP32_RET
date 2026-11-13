/*
 * config.h
 *
 * allows the user to configure static parameters.
 *
 * Note: Make sure with all pin defintions of your hardware that each pin number is
 *       only defined once.

 Copyright (c) 2013-2018 Collin Kidder, Michael Neuweiler, Charles Galpin

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *      Author: Michael Neuweiler
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <WiFi.h>
#include <Preferences.h>
#include <ESP32-TWAI-CAN.hpp>

// define GPIO pins for ESP32 internal CAN bus controller
#define GPIO_CAN_TX     D5  //xiao_esp32c6 board
#define GPIO_CAN_RX     D4  //xiao_esp32c6 board
//#define GPIO_CAN_TX     GPIO_NUM_17     //esp32, esp32s3 boards
//#define GPIO_CAN_RX     GPIO_NUM_16     //esp32, esp32s3 boards

// set CAN bus speed
#define CAN_BUS_SPEED   500000  //bps

// set USB Serial baud rate
#define SERIAL_BAUD_RATE    1000000     //bps

// define SSID and password for SoftAP
#define SSID_NAME "ESP32_RET"
#define WPA2KEY "aBigSecret"

// ESP32 dev module built in LED
//#define ESP32_BUILTIN_LED 2

//size to use for buffering writes to USB. On the ESP32 we're actually talking TTL serial to a TTL<->USB chip
#define SER_BUFF_SIZE       1024

//Buffer for CAN frames when sending over wifi. This allows us to build up a multi-frame packet that goes
//over the air all at once. This is much more efficient than trying to send a new TCP/IP packet for each and every
//frame. It delays frames from getting to the other side a bit but that's life.
//Probably don't set this over 2048 as the default packet size for wifi is 2312 including all overhead.
#define WIFI_BUFF_SIZE      1024

//Number of microseconds between hard flushes of the serial buffer (if not in wifi mode) or the wifi buffer (if in wifi mode)
//This keeps the latency more consistent. Otherwise the buffer could partially fill and never send.
#define SER_BUFF_FLUSH_INTERVAL 20000

#define CFG_BUILD_NUM   001

#define CFG_VERSION "ESP32_RET v0.1 (Oct 22, 2026)"
#define PREF_NAME   "ESP32_RET"

#define NUM_BUSES   1   //max # of buses supported by any of the supported boards

//Slows down the blinks to make them more visible
#define BLINK_SLOWNESS  100 

//How many devices to allow to connect to our WiFi telnet port?
#define MAX_CLIENTS 1

struct FILTER {  //should be 10 bytes
    uint32_t id;
    uint32_t mask;
    boolean extended;
    boolean enabled;
};

struct EEPROMSettings {
    uint32_t CAN0Speed;
    boolean CAN0_Enabled;
    boolean CAN0ListenOnly; //if true we don't allow any messing with the bus but rather just passively monitor.

    boolean useBinarySerialComm; //use a binary protocol on the serial link or human readable format?

    uint8_t logLevel;   //Level of logging to output on serial line
    uint8_t systemType; //0 = ESP32 Board, maybe others in the future
    
    boolean enableBT;   //are we enabling bluetooth too?
    char btName[32];

    //if we're using WiFi then output to serial is disabled (it's far too slow to keep up)  
    uint8_t wifiMode;  //0 = don't use wifi, 1 = connect to an AP, 2 = Create an AP
    char SSID[32];     //Null terminated string for the SSID
    char WPA2Key[64];  //Null terminated string for the key. Can be a passphase or the actual key
};

struct SystemSettings {
    uint8_t LED_CANTX;
    uint8_t LED_CANRX;
    uint8_t LED_LOGGING;
    boolean txToggle; //LED toggle values
    boolean rxToggle;
    boolean logToggle;
    int8_t numBuses; //number of buses this hardware currently supports.
    WiFiClient clientNodes[MAX_CLIENTS];
    boolean isWifiConnected;
    boolean isWifiActive;
};

class GVRET_Comm_Handler;
class SerialConsole;
class CANManager;


extern EEPROMSettings settings;
extern SystemSettings SysSettings;
extern Preferences nvPrefs;
extern GVRET_Comm_Handler serialGVRET;
extern GVRET_Comm_Handler wifiGVRET;
extern SerialConsole console;
extern CANManager canManager;

#endif /* CONFIG_H_ */
