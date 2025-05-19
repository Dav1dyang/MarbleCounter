# Seven-Segment Display Wiring with SevSeg Library

## Library Installation

Before uploading the code, install the SevSeg library in Arduino IDE:

1. In Arduino IDE, go to **Tools → Manage Libraries...**
2. Search for "SevSeg"
3. Install "SevSeg" by Dean Reading (or the latest version available)

## Connection Diagram

The 7-segment display is connected to the Xiao ESP32S3 and controlled using the SevSeg library:

### Segment Connections (to control which segments light up)
- Segment A → GPIO 0 (D0)
- Segment B → GPIO 1 (D1)
- Segment C → GPIO 2 (D2)
- Segment D → GPIO 3 (D3)
- Segment E → GPIO 4 (D4)
- Segment F → GPIO 5 (D5)
- Segment G → GPIO 6 (D6)
- Decimal Point → GPIO 9 (D9)

### Digit Connections (3-digit display)
- Digit 1 (leftmost/hundreds) → GPIO 10 (D10)
- Digit 2 (tens) → GPIO 7 (D7)
- Digit 3 (rightmost/ones) → GPIO 8 (D8)

### Other Connections
- Built-in LED → LED_BUILTIN (used for visual feedback)

## Important Notes

1. The SevSeg library simplifies control of the 7-segment display and handles all the multiplexing internally.

2. This setup is configured for a 3-digit display with a maximum value of 999.

3. This wiring assumes a **common cathode** 7-segment display. If you're using a **common anode** display, change the hardwareConfig parameter in the code to COMMON_ANODE:
   ```arduino
   byte hardwareConfig = COMMON_ANODE;
   ```

4. Current-limiting resistors (220-330 ohms) are still needed for each segment connection.

5. The decimal point is connected to D9 but is not used in the current code implementation.

## Pin Order Note

The digit pins order in the code is important and should match your physical connection:
```arduino
byte digitPins[] = {D10, D7, D8}; // Hundreds, Tens, Ones
```

## Troubleshooting

1. **Dim Display**: Adjust the brightness using `sevseg.setBrightness(90)` where the value is between 0-100.

2. **Flickering Display**: This may be caused by insufficient refresh rate. Make sure `sevseg.refreshDisplay()` is called frequently with minimal delays in the main loop.

3. **Incorrect Digits**: Verify the digit pins array in the code matches your physical wiring order. 