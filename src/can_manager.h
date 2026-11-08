#pragma once
#include <ESP32-TWAI-CAN.hpp>
#include "config.h"

typedef struct {
    uint32_t bitsPerQuarter;
    uint32_t bitsSoFar;
    uint8_t busloadPercentage;
} BUSLOAD;


class CANManager
{
public:
    CANManager();
    void addBits(int offset, CanFrame &frame);
    void sendFrame(TwaiCAN *bus, CanFrame &frame);
    void displayFrame(CanFrame &frame, int whichBus);
    void loop();
    void setup();

private:
    BUSLOAD busLoad[NUM_BUSES];
    uint32_t busLoadTimer;
};
