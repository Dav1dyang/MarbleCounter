# Marble Counter Project

This project uses two ESP32-based controllers to count marbles as they drop into a container and display the count on a seven-segment display.

## Components

### Sender Unit (Marble Detector)
- Adafruit QT Py ESP32 Pico
- MSA301 Accelerometer (connected via STEMMA QT)
- Battery Power Supply

### Receiver Unit (Display)
- Xiao ESP32-S3
- 3-digit 7-segment display (common cathode)
- Battery Power Supply

## Wiring Instructions

### Sender Unit (Marble Detector)
1. **MSA301 to QT Py ESP32 Pico:**
   - Connect via the STEMMA QT / Qwiic connector using a STEMMA QT cable
   - The code is configured to use the SDA1/SCL1 pins through the STEMMA QT connector
   - If using separate wires instead of the STEMMA QT connector:
     - VIN → 3.3V
     - GND → GND
     - SCL → SCL1 pin
     - SDA → SDA1 pin

### Receiver Unit (Display)
1. **7-Segment Display to Xiao ESP32-S3:**
   - Display segment A → GPIO 0 (D0)
   - Display segment B → GPIO 1 (D1)
   - Display segment C → GPIO 2 (D2)
   - Display segment D → GPIO 3 (D3)
   - Display segment E → GPIO 4 (D4)
   - Display segment F → GPIO 5 (D5)
   - Display segment G → GPIO 6 (D6)
   - Display segment DP → GPIO 9 (D9) (optional)

2. **7-Segment Display Digit Pins to Xiao ESP32-S3:**
   - Digit 1 (leftmost/hundreds) → GPIO 10 (D10)
   - Digit 2 (tens) → GPIO 7 (D7)
   - Digit 3 (rightmost/ones) → GPIO 8 (D8)

## Software Setup

1. Install the required libraries:
   - For the QT Py ESP32 Pico (Sender):
     - Adafruit MSA301 Library
     - Adafruit Sensor Library
     - ESP32 WiFi Library
     - ESP-NOW Library

   - For the Xiao ESP32-S3 (Receiver):
     - ESP32 WiFi Library
     - ESP-NOW Library

2. Configure the ESP-NOW MAC Address:
   - Upload the "GetMacAddress/GetMacAddress.ino" sketch to the Xiao ESP32-S3
   - Note the MAC address from the Serial Monitor
   - Update the `receiverMacAddress` array in the MarbleDetector/MarbleDetector.ino with this MAC address

3. Upload the respective code:
   - Upload "Sender/MarbleDetector/MarbleDetector.ino" to the QT Py ESP32 Pico
   - Upload "Receiver/DisplayController/DisplayController.ino" to the Xiao ESP32-S3

## STEMMA QT Connection

This project uses the STEMMA QT connector on the QT Py ESP32 Pico to connect to the MSA301 accelerometer. The code configures the default Wire interface to use SDA1/SCL1 pins, which correspond to the STEMMA QT connector:

```arduino
// Initialize the MSA301 sensor
Adafruit_MSA301 msa;

void setup() {
  // Initialize Wire with the STEMMA QT connector pins
  Wire.begin(SDA1, SCL1);
  
  // Initialize the MSA301 sensor using default Wire
  if (!msa.begin()) {
    // Error handling
  }
}
```

This approach configures the default I2C bus to use the STEMMA QT connector pins.

## Detection Method

The project uses tap detection functionality built into the MSA301 accelerometer. This approach leverages the accelerometer's internal tap detection algorithm with a configurable threshold for sensitivity. The code uses debouncing to prevent multiple counts from a single marble.

The sensitivity can be fine-tuned by adjusting the `tapThreshold` value in the code (lower value = more sensitive). The debounce time can also be adjusted via the `TAP_DEBOUNCE_TIME` constant to optimize detection for your specific container and marble size/weight.

## Folder Structure
For proper Arduino IDE compatibility, the project is organized as follows:
```
├── README.md
├── Receiver/
│   └── DisplayController/
│       └── DisplayController.ino
├── Sender/
│   └── MarbleDetector/
│       └── MarbleDetector.ino
├── Tests/
│   ├── MSA301_Test/
│   │   └── MSA301_Test.ino
│   ├── SevenSegment_Test/
│   │   └── SevenSegment_Test.ino
│   ├── MSA301_ESP32NOW_Test/
│   │   └── MSA301_ESP32NOW_Test.ino
│   └── Display_ESP32NOW_Test/
│       └── Display_ESP32NOW_Test.ino
└── Utilities/
    └── GetMacAddress/
        └── GetMacAddress.ino
```

## AI-Assisted Development

This project was developed with the assistance of AI tools. The code, documentation, and troubleshooting steps were created collaboratively using AI assistance, which helped with:

- Generating initial code structure and algorithms
- Debugging and optimizing sensor detection methods
- Creating clear documentation and wiring instructions
- Simplifying the display controller implementation by removing the multiplexer
- Implementing ESP-NOW communication between the devices

The AI assistance accelerated development time while ensuring best practices were followed in the code implementation.

## Usage

1. Secure the Sender unit to the side of the container where marbles will drop
2. Place the container on a stable surface
3. Drop marbles into the container and watch the count update on the display

## Troubleshooting

- **No communication between units:**
  - Verify the MAC address is correctly set
  - Make sure both devices are powered on
  - Try resetting both devices

- **False triggers or missed marbles:**
  - Adjust the tap threshold in the code (lower value = more sensitive)
  - Try repositioning the accelerometer on the container
  - Make sure the container is on a stable surface
  - Consider adjusting the debounce time if needed

- **Display issues:**
  - Check all wiring connections between the ESP32 and the display
  - Verify the common cathode/common anode configuration of your display (code is for common cathode)

- **I2C Communication Issues:**
  - Make sure the STEMMA QT connector is securely connected
  - If using direct wires instead of STEMMA QT connector, double-check the connections to SDA1 and SCL1 pins
  - Try reducing the I2C frequency if connection is unstable (you can add `Wire.setClock(100000);` after the begin() call to set it to 100kHz)

## Customization

- You can adjust the pin assignments in the code to match your specific hardware setup
- Modify the `tapThreshold` and `TAP_DEBOUNCE_TIME` values in the MarbleDetector code to optimize for your specific container and marbles

## Files

- **Tests/MSA301_Test/MSA301_Test.ino:** Test sketch for the accelerometer using tap detection
- **Tests/SevenSegment_Test/SevenSegment_Test.ino:** Test sketch for the seven-segment display
- **Tests/MSA301_ESP32NOW_Test/MSA301_ESP32NOW_Test.ino:** Test sketch for the accelerometer with ESP-NOW communication
- **Tests/Display_ESP32NOW_Test/Display_ESP32NOW_Test.ino:** Test sketch for the display with ESP-NOW communication
- **Sender/MarbleDetector/MarbleDetector.ino:** Main code for the marble detector unit
- **Receiver/DisplayController/DisplayController.ino:** Main code for the display controller unit
- **Utilities/GetMacAddress/GetMacAddress.ino:** Utility to get the ESP32's MAC address 