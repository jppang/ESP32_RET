# ESP32_RET
A slimmed down version of firmware for ESP32-like MCUs that can connect to the SavvyCAN application. 

This firmware directly inherits from the [ESP32RET](https://github.com/collin80/ESP32RET) and [ESP32_RET_SD](https://github.com/MotorvateDIY/ESP32_RET_SD) projects. However, in order to make it as simple as possible only those core functions which are connecting to SavvyCAN application by USB and WiFi are retained. Besides, the core connection libraries dependancy are substituted from [can_common](https://github.com/collin80/can_common) and [esp32_can](https://github.com/collin80/esp32_can) to [ESP32-TWAI-CAN](https://github.com/handmade0octopus/ESP32-TWAI-CAN). The reason I switch to the ESP32-TWAI-CAN library is just because I encountered a rebooting loop while successfully compiled and uploaded the firmware to MCUs. I have tried ESP32 30pin and Seeed XIAO ESP32C6 MCUs.

This firmware is developed in the VS Code with Platformio environment.
