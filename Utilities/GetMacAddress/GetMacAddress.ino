/*
 * Get MAC Address Utility
 *
 * Run this on the Xiao ESP32-S3 to get its MAC address
 * Use this MAC address in the MarbleDetector.ino sketch
 */

#include <WiFi.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10); // Wait for serial port to open

    Serial.println("\nESP32 MAC Address Finder");

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Get and print MAC address
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());

    // Print in array format for easy copy-paste
    uint8_t mac[6];
    WiFi.macAddress(mac);
    Serial.print("Copy this into receiverMacAddress: {0x");
    Serial.print(mac[0], HEX);
    Serial.print(", 0x");
    Serial.print(mac[1], HEX);
    Serial.print(", 0x");
    Serial.print(mac[2], HEX);
    Serial.print(", 0x");
    Serial.print(mac[3], HEX);
    Serial.print(", 0x");
    Serial.print(mac[4], HEX);
    Serial.print(", 0x");
    Serial.print(mac[5], HEX);
    Serial.println("};");
}

void loop()
{
    // Nothing to do here
    delay(10000);
}