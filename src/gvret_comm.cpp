/*
Implements handling of the GVRET comm protocol, both sending and receiving
*/

#include "gvret_comm.h"
#include "SerialConsole.h"
#include "config.h"
#include "can_manager.h"

GVRET_Comm_Handler::GVRET_Comm_Handler()
{
    step = 0;
    state = IDLE;
    transmitBufferLength = 0;
}

void GVRET_Comm_Handler::processIncomingByte(uint8_t in_byte)
{
    uint32_t busSpeed = 0;
    uint32_t now = micros();

    uint8_t temp8;
    uint16_t temp16;

    switch (state)
    {
    case IDLE:
        if (in_byte == 0xF1)
        {
            state = GET_COMMAND;
        }
        else if (in_byte == 0xE7)
        {
            settings.useBinarySerialComm = true;
        }
        else
        {
            console.rcvCharacter((uint8_t)in_byte);
        }
        break;
    case GET_COMMAND:
        switch (in_byte)
        {
        case PROTO_BUILD_CAN_FRAME:
            state = BUILD_CAN_FRAME;
            buff[0] = 0xF1;
            step = 0;
            break;
        case PROTO_TIME_SYNC:
            state = TIME_SYNC;
            step = 0;
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 1; // time sync
            transmitBuffer[transmitBufferLength++] = (uint8_t)(now & 0xFF);
            transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 8);
            transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 16);
            transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 24);
            break;
        case PROTO_DIG_INPUTS:
            // immediately return the data for digital inputs
            temp8 = 0; // getDigital(0) + (getDigital(1) << 1) + (getDigital(2) << 2) + (getDigital(3) << 3) + (getDigital(4) << 4) + (getDigital(5) << 5);
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 2; // digital inputs
            transmitBuffer[transmitBufferLength++] = temp8;
            temp8 = checksumCalc(buff, 2);
            transmitBuffer[transmitBufferLength++] = temp8;
            state = IDLE;
            break;
        case PROTO_ANA_INPUTS:
            // immediately return data on analog inputs
            temp16 = 0; // getAnalog(0);  // Analogue input 1
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 3;
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(1);  // Analogue input 2
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(2);  // Analogue input 3
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(3);  // Analogue input 4
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(4);  // Analogue input 5
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(5);  // Analogue input 6
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp16 = 0; // getAnalog(6);  // Vehicle Volts
            transmitBuffer[transmitBufferLength++] = temp16 & 0xFF;
            transmitBuffer[transmitBufferLength++] = uint8_t(temp16 >> 8);
            temp8 = checksumCalc(buff, 9);
            transmitBuffer[transmitBufferLength++] = temp8;
            state = IDLE;
            break;
        case PROTO_SET_DIG_OUT:
            state = SET_DIG_OUTPUTS;
            buff[0] = 0xF1;
            break;
        case PROTO_SETUP_CANBUS:
            state = SETUP_CANBUS;
            step = 0;
            buff[0] = 0xF1;
            break;
        case PROTO_GET_CANBUS_PARAMS:
            // immediately return data on canbus params
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 6;
            transmitBuffer[transmitBufferLength++] = settings.CAN0_Enabled + ((unsigned char)settings.CAN0ListenOnly << 4);
            transmitBuffer[transmitBufferLength++] = settings.CAN0Speed;
            transmitBuffer[transmitBufferLength++] = settings.CAN0Speed >> 8;
            transmitBuffer[transmitBufferLength++] = settings.CAN0Speed >> 16;
            transmitBuffer[transmitBufferLength++] = settings.CAN0Speed >> 24;
            state = IDLE;
            break;
        case PROTO_GET_DEV_INFO:
            // immediately return device information
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 7;
            transmitBuffer[transmitBufferLength++] = CFG_BUILD_NUM & 0xFF;
            transmitBuffer[transmitBufferLength++] = (CFG_BUILD_NUM >> 8);
            transmitBuffer[transmitBufferLength++] = 0x20;
            transmitBuffer[transmitBufferLength++] = 0;
            transmitBuffer[transmitBufferLength++] = 0;
            transmitBuffer[transmitBufferLength++] = 0; // was single wire mode. Should be rethought for this board.
            state = IDLE;
            break;
        case PROTO_SET_SW_MODE:
            buff[0] = 0xF1;
            state = SET_SINGLEWIRE_MODE;
            step = 0;
            break;
        case PROTO_KEEPALIVE:
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 0x09;
            transmitBuffer[transmitBufferLength++] = 0xDE;
            transmitBuffer[transmitBufferLength++] = 0xAD;
            state = IDLE;
            break;
        case PROTO_SET_SYSTYPE:
            buff[0] = 0xF1;
            state = SET_SYSTYPE;
            step = 0;
            break;
        case PROTO_ECHO_CAN_FRAME:
            state = ECHO_CAN_FRAME;
            buff[0] = 0xF1;
            step = 0;
            break;
        case PROTO_GET_NUMBUSES:
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 12;
            transmitBuffer[transmitBufferLength++] = SysSettings.numBuses;
            state = IDLE;
            break;
        case PROTO_GET_EXT_BUSES:
            transmitBuffer[transmitBufferLength++] = 0xF1;
            transmitBuffer[transmitBufferLength++] = 13;
            for (int u = 2; u < 17; u++)
                transmitBuffer[transmitBufferLength++] = 0;
            step = 0;
            state = IDLE;
            break;
        case PROTO_SET_EXT_BUSES:
            state = SETUP_EXT_BUSES;
            step = 0;
            buff[0] = 0xF1;
            break;
        }
        break;
    case BUILD_CAN_FRAME:
        buff[1 + step] = in_byte;
        switch (step)
        {
        case 0:
            build_out_frame.identifier = in_byte;
            break;
        case 1:
            build_out_frame.identifier |= in_byte << 8;
            break;
        case 2:
            build_out_frame.identifier |= in_byte << 16;
            break;
        case 3:
            build_out_frame.identifier |= in_byte << 24;
            if (build_out_frame.identifier & 1 << 31)
            {
                build_out_frame.identifier &= 0x7FFFFFFF;
                build_out_frame.extd = true;
            }
            else
                build_out_frame.extd = false;
            break;
        case 4:
            out_bus = in_byte & 3;
            break;
        case 5:
            build_out_frame.data_length_code = in_byte & 0xF;
            if (build_out_frame.data_length_code > 8)
            {
                build_out_frame.data_length_code = 8;
            }
            break;
        default:
            if (step < build_out_frame.data_length_code + 6)
            {
                build_out_frame.data[step - 6] = in_byte;
            }
            else
            {
                state = IDLE;
                build_out_frame.rtr = 0;
                if (out_bus == 0)
                    canManager.sendFrame(&CAN0, build_out_frame);
            }
            break;
        }
        step++;
        break;
    case TIME_SYNC:
        state = IDLE;
        break;
    case GET_DIG_INPUTS:
        // nothing to do
        break;
    case GET_ANALOG_INPUTS:
        // nothing to do
        break;
    case SET_DIG_OUTPUTS: // todo: validate the XOR byte
        break;
    case SETUP_CANBUS: // todo: validate checksum
        switch (step)
        {
        case 0:
            build_int = in_byte;
            break;
        case 1:
            build_int |= in_byte << 8;
            break;
        case 2:
            build_int |= in_byte << 16;
            break;
        case 3:
            build_int |= in_byte << 24;
            busSpeed = build_int & 0xFFFFF;
            if (busSpeed > 1000000)
                busSpeed = 1000000;

            if (build_int > 0)
            {
                if (build_int & 0x80000000ul) // signals that enabled and listen only status are also being passed
                {
                    if (build_int & 0x40000000ul)
                    {
                        settings.CAN0_Enabled = true;
                    }
                    else
                    {
                        settings.CAN0_Enabled = false;
                    }
                    if (build_int & 0x20000000ul)
                    {
                        settings.CAN0ListenOnly = true;
                    }
                    else
                    {
                        settings.CAN0ListenOnly = false;
                    }
                }
                else
                {
                    // if not using extended status mode then just default to enabling - this was old behavior
                    settings.CAN0_Enabled = true;
                }
                settings.CAN0Speed = busSpeed;
            }
            else
            { // disable first canbus
                settings.CAN0_Enabled = false;
            }

            if (settings.CAN0_Enabled)
            {
                CAN0.begin(CAN0.convertSpeed(uint16_t (settings.CAN0Speed / 1000)));
                if (settings.CAN0ListenOnly)
                    ;
                else
                    ;
            }
            else
                CAN0.end();
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            state = IDLE;
            break;
        }
        step++;
        break;
    case GET_CANBUS_PARAMS:
        // nothing to do
        break;
    case GET_DEVICE_INFO:
        // nothing to do
        break;
    case SET_SINGLEWIRE_MODE:
        if (in_byte == 0x10)
        {
        }
        else
        {
        }
        state = IDLE;
        break;
    case SET_SYSTYPE:
        settings.systemType = in_byte;
        state = IDLE;
        break;
    case ECHO_CAN_FRAME:
        buff[1 + step] = in_byte;
        switch (step)
        {
        case 0:
            build_out_frame.identifier = in_byte;
            break;
        case 1:
            build_out_frame.identifier |= in_byte << 8;
            break;
        case 2:
            build_out_frame.identifier |= in_byte << 16;
            break;
        case 3:
            build_out_frame.identifier |= in_byte << 24;
            if (build_out_frame.identifier & 1 << 31)
            {
                build_out_frame.identifier &= 0x7FFFFFFF;
                build_out_frame.extd = true;
            }
            else
                build_out_frame.extd = false;
            break;
        case 4:
            out_bus = in_byte & 1;
            break;
        case 5:
            build_out_frame.data_length_code = in_byte & 0xF;
            if (build_out_frame.data_length_code > 8)
                build_out_frame.data_length_code = 8;
            break;
        default:
            if (step < build_out_frame.data_length_code + 6)
            {
                build_out_frame.data[step - 6] = in_byte;
            }
            else
            {
                state = IDLE;
                canManager.displayFrame(build_out_frame, 0);
            }
            break;
        }
        step++;
        break;
    case SETUP_EXT_BUSES: // setup enable/listenonly/speed for SWCAN, Enable/Speed for LIN1, LIN2
        switch (step)
        {
        case 0:
            build_int = in_byte;
            break;
        case 1:
            build_int |= in_byte << 8;
            break;
        case 2:
            build_int |= in_byte << 16;
            break;
        case 3:
            build_int |= in_byte << 24;
            break;
        case 4:
            build_int = in_byte;
            break;
        case 5:
            build_int |= in_byte << 8;
            break;
        case 6:
            build_int |= in_byte << 16;
            break;
        case 7:
            build_int |= in_byte << 24;
            break;
        case 8:
            build_int = in_byte;
            break;
        case 9:
            build_int |= in_byte << 8;
            break;
        case 10:
            build_int |= in_byte << 16;
            break;
        case 11:
            build_int |= in_byte << 24;
            state = IDLE;
            break;
        }
        step++;
        break;
    }
}

void GVRET_Comm_Handler::sendFrameToBuffer(CanFrame &frame, int whichBus)
{
    uint8_t temp;
    size_t writtenBytes;

    if (settings.useBinarySerialComm)
    {
        if (frame.extd)
            frame.identifier |= 1 << 31;
        transmitBuffer[transmitBufferLength++] = 0xF1;
        transmitBuffer[transmitBufferLength++] = 0; // 0 = canbus frame sending
        uint32_t now = micros();
        transmitBuffer[transmitBufferLength++] = (uint8_t)(now & 0xFF);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 8);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 16);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(now >> 24);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(frame.identifier & 0xFF);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(frame.identifier >> 8);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(frame.identifier >> 16);
        transmitBuffer[transmitBufferLength++] = (uint8_t)(frame.identifier >> 24);
        transmitBuffer[transmitBufferLength++] = frame.data_length_code + (uint8_t)(whichBus << 4);
        for (int c = 0; c < frame.data_length_code; c++)
        {
            transmitBuffer[transmitBufferLength++] = frame.data[c];
        }
        temp = 0;
        transmitBuffer[transmitBufferLength++] = temp;
        // Serial.write(buff, 12 + frame.length);
    }
    else
    {

        // SavvyCAN CSV format example
        // Time Stamp,ID,Extended,Dir,Bus,LEN,D1,D2,D3,D4,D5,D6,D7,D8
        // 8740939,000666,false,Rx,0,8,00,03,00,00,00,00,00,00,

        writtenBytes = sprintf((char *)&transmitBuffer[transmitBufferLength], "%d - %X", micros(), frame.identifier);

        transmitBufferLength += writtenBytes;
        if (frame.extd)
        {
            sprintf((char *)&transmitBuffer[transmitBufferLength], " X ");
            transmitBufferLength += 3;
        }
        else
        {
            sprintf((char *)&transmitBuffer[transmitBufferLength], " S ");
            transmitBufferLength += 3;
        }

        writtenBytes = sprintf((char *)&transmitBuffer[transmitBufferLength], "%i %i", whichBus, frame.data_length_code);
        transmitBufferLength += writtenBytes;
        for (int c = 0; c < frame.data_length_code; c++)
        {
            writtenBytes = sprintf((char *)&transmitBuffer[transmitBufferLength], " %x", frame.data[c]);
            transmitBufferLength += writtenBytes;
        }
        // no need for CR and LF, just LF for SavvyCAN to import data
        sprintf((char *)&transmitBuffer[transmitBufferLength], "\n");
        transmitBufferLength += 1;
    }
}

size_t GVRET_Comm_Handler::numAvailableBytes()
{
    return transmitBufferLength;
}

void GVRET_Comm_Handler::clearBufferedBytes()
{
    transmitBufferLength = 0;
}

uint8_t *GVRET_Comm_Handler::getBufferedBytes()
{
    return transmitBuffer;
}

// Get the value of XOR'ing all the bytes together. This creates a reasonable checksum that can be used
// to make sure nothing too stupid has happened on the comm.
uint8_t GVRET_Comm_Handler::checksumCalc(uint8_t *buffer, int length)
{
    uint8_t value = 0;
    for (int c = 0; c < length; c++)
    {
        value ^= buffer[c];
    }
    return value;
}