# Pulse-Oximeter-Senior-Design
ESP32-based pulse oximeter using red and infrared PPG signals for SpO₂ and heart-rate estimation.


This program is intended to be used with a MAX30105 sensor and a ESP32 microcontroller.

## Software Installation and Setup

The pulse oximeter firmware was developed using the Arduino Integrated Development Environment (IDE) and programmed onto an ESP32 microcontroller. To compile and upload the software, the appropriate board support package and required libraries must be installed.

### 1. Install Arduino IDE
Download and install the latest compatible version of Arduino IDE from https://www.arduino.cc/en/software.

### 2. Install ESP32 Board Support
1. Open Arduino IDE and navigate to **File → Preferences**.
2. Under **Additional Boards Manager URLs**, add the following URL:

   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

3. Navigate to **Tools → Board → Boards Manager**.
4. Search for **esp32**.
5. Install **esp32 by Espressif Systems**.
6. Select the appropriate ESP32 board configuration under **Tools → Board**.

### 3. Install Required Libraries
1. Navigate to **Sketch → Include Library → Manage Libraries**.
2. Search for **SparkFun MAX3010x Sensor Library**.
3. Install the library developed by SparkFun Electronics.

The library provides the functions required to configure and communicate with the MAX30105 optical sensor. The firmware also uses Maxim Integrated's SpO₂ and heart-rate algorithm, which is included in the SparkFun MAX3010x Sensor Library.

### 4. Configure the Microcontroller Connection
1. Connect the ESP32 programming interface to the computer using a USB cable.
2. Install the appropriate USB-to-UART driver if the computer does not recognize the device.
3. In Arduino IDE, navigate to **Tools → Port** and select the corresponding COM port.
4. Verify that the correct ESP32 board configuration has been selected.

### 5. Compile and Upload the Firmware
1. Download the source code from this repository.
2. Open `pulseox.ino` in Arduino IDE.
3. Verify that the required libraries are installed and that the sensor connections match the firmware configuration.
4. Click **Verify** to compile the program.
5. Click **Upload** to transfer the firmware to the microcontroller.
6. Open the **Serial Monitor** and set the baud rate to **115200** to observe the SpO₂ and heart-rate measurements.

### Code Attribution
The firmware incorporates source code from the SparkFun MAX3010x Sensor Library and the Maxim Integrated SpO₂ algorithm. Additional functionality was developed with AI assistance and subsequently reviewed and debugged by the project team.
