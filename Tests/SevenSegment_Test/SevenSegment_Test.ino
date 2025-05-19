/*
 * Seven Segment Display Test using SevSeg Library
 * For Xiao ESP32-S3
 */

#include <SevSeg.h>

// Create an instance of the SevSeg object
SevSeg sevseg;

// Test counter
int testNumber = 0;
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 1000; // Update every second

void setup()
{
    Serial.begin(115200);
    Serial.println("Seven Segment Display Test with SevSeg Library");

    // Configure the display
    byte numDigits = 3;
    byte digitPins[] = {D10, D7, D8};                      // Common pins D7-D10
    byte segmentPins[] = {D0, D1, D2, D3, D4, D5, D9, D6}; // Segments A-G on D0-D6
    bool resistorsOnSegments = true;                       // Use when resistors are on segment pins
    bool updateWithDelays = false;                         // Use interrupts for display multiplexing
    byte hardwareConfig = COMMON_CATHODE;                  // COMMON_CATHODE or COMMON_ANODE

    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays);
    sevseg.setBrightness(90); // Set brightness level (0-100)

    // Display test pattern (all segments on)
    sevseg.refreshDisplay();
    delay(1000);
}

void loop()
{
    // Update test number every second
    if (millis() - lastUpdateTime >= updateInterval)
    {
        lastUpdateTime = millis();
        testNumber++;
        if (testNumber > 999)
            testNumber = 0;

        Serial.print("Displaying number: ");
        Serial.println(testNumber);

        // Update display with new number
        sevseg.setNumber(testNumber, 0); // Second parameter is decimal points (0 = none)
    }

    // Must run continuously to keep display updated
    sevseg.refreshDisplay();
}