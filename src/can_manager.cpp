#include <Arduino.h>
#include "can_manager.h"
#include <ESP32-TWAI-CAN.hpp>
#include "config.h"
#include "SerialConsole.h"
#include "gvret_comm.h"

uint8_t CAN_frame_count;

CANManager::CANManager()
{
}

void CANManager::setup()
{
    if (settings.CAN0_Enabled)
    {
        if (settings.systemType == 0)
        {
            CAN0.setPins(GPIO_CAN_TX, GPIO_CAN_RX);
            CAN0.begin(CAN0.convertSpeed(uint16_t (CAN_BUS_SPEED / 1000)));
            if(CAN0.canState() == 1) {
                Serial.print("\nCAN enabled at: ");
                Serial.print(CAN_BUS_SPEED);
                Serial.println(" bits/sec"); 
            } else {
                Serial.println("\nCAN failed to start. ");
            }
            
        }
    }
    else
    {
        CAN0.end();
    }
}

void CANManager::addBits(int offset, CanFrame &frame)
{
    if (offset < 0)
        return;
    if (offset >= NUM_BUSES)
        return;
    busLoad[offset].bitsSoFar += 41 + (frame.data_length_code * 9);
    if (frame.extd)
        busLoad[offset].bitsSoFar += 18;
}

void CANManager::sendFrame(TwaiCAN *bus, CanFrame &frame)
{
    int whichBus;
    if (bus == &CAN0)
        whichBus = 0;
    bus->writeFrame(frame);
    addBits(whichBus, frame);
}

void CANManager::displayFrame(CanFrame &frame, int whichBus)
{
    if (SysSettings.isWifiActive)
        wifiGVRET.sendFrameToBuffer(frame, whichBus);
    else
        serialGVRET.sendFrameToBuffer(frame, whichBus);
}

void CANManager::loop()
{
    CanFrame incoming;
    size_t wifiLength = wifiGVRET.numAvailableBytes();
    size_t serialLength = serialGVRET.numAvailableBytes();
    size_t maxLength = (wifiLength > serialLength) ? wifiLength : serialLength;
    while (CAN0.inRxQueue() > 0 && (maxLength < (WIFI_BUFF_SIZE - 80)))
    {
        if(CAN0.readFrame(incoming)) {
            addBits(0, incoming);
            displayFrame(incoming, 0);
            wifiLength = wifiGVRET.numAvailableBytes();
            serialLength = serialGVRET.numAvailableBytes();
            maxLength = (wifiLength > serialLength) ? wifiLength : serialLength;
        }
        delay(250);  
    }
}
