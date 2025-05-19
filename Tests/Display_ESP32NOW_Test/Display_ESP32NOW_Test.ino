/*
 * ESP32_NOW Receiver + Seven Segment Display Test
 * For Xiao ESP32-S3
 *
 * This test receives marble counts via ESP32_NOW broadcast
 * and displays them on a 7-segment display.
 * Simplified version for testing.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <SevSeg.h>
#include <algorithm> // For std::min

// Create an instance of the SevSeg object
SevSeg sevseg;

// LED for visual feedback
const int LED_PIN = LED_BUILTIN;

// Counter variable
uint32_t marbleCount = 0;
bool newDataReceived = false;

// ESP-NOW settings
#define ESPNOW_WIFI_CHANNEL 1

// Callback function when data is received via ESP-NOW
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    char recv_data[32] = {0};

    // Copy only what we need to avoid buffer overflows
    size_t copy_len = std::min(static_cast<size_t>(len), sizeof(recv_data) - 1);
    memcpy(recv_data, data, copy_len);

    Serial.printf("Received data: %s\n", recv_data);

    // Check if it's a marble count message
    if (strncmp(recv_data, "MARBLE:", 7) == 0)
    {
        // Extract count from the message
        uint32_t count = strtoul(&recv_data[7], NULL, 10);
        Serial.printf("Marble count: %lu\n", count);

        // Update our count
        marbleCount = count;
        if (marbleCount > 999)
            marbleCount = 999; // Limit to 3 digits

        // Flag for display update
        newDataReceived = true;

        // Visual feedback for data reception
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500); // Give serial time to initialize

    Serial.println("ESP32_NOW + Seven Segment Display Test");

    // Configure the display
    byte numDigits = 3;
    byte digitPins[] = {D10, D7, D8};                      // Common pins in order
    byte segmentPins[] = {D0, D1, D2, D3, D4, D5, D9, D6}; // Segments A-G and DP
    bool resistorsOnSegments = true;                       // Use when resistors are on segment pins
    bool updateWithDelays = false;                         // Use interrupts for display multiplexing
    byte hardwareConfig = COMMON_CATHODE;                  // COMMON_CATHODE or COMMON_ANODE

    Serial.println("Initializing SevSeg...");
    sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays);
    sevseg.setBrightness(90); // Set brightness level (0-100)
    Serial.println("SevSeg initialized");

    // Setup LED for visual feedback
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize WiFi for ESP-NOW
    Serial.println("Setting up WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.setChannel(ESPNOW_WIFI_CHANNEL);

    // Wait for WiFi to initialize
    int attempts = 0;
    while (!WiFi.status() == WL_CONNECTED && attempts < 10)
    {
        Serial.println("Waiting for WiFi...");
        delay(100);
        attempts++;
    }

    Serial.println("WiFi initialized");
    Serial.println("  Mode: STA");
    Serial.println("  MAC Address: " + WiFi.macAddress());
    Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

    // Initialize ESP-NOW
    Serial.println("Setting up ESP-NOW...");
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        while (1)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink to indicate error
            delay(300);
            sevseg.refreshDisplay(); // Keep display working
        }
    }
    Serial.println("ESP-NOW initialized");

    // Register callback function for receiving data
    if (esp_now_register_recv_cb(OnDataRecv) != ESP_OK)
    {
        Serial.println("Error registering receive callback");
        while (1)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink to indicate error
            delay(200);
            sevseg.refreshDisplay(); // Keep display working
        }
    }
    Serial.println("Callback registered");

    // Initial display test - all segments on
    sevseg.refreshDisplay();
    delay(1000);

    // Show initial count (0)
    sevseg.setNumber(marbleCount, 0);
    sevseg.refreshDisplay();

    Serial.println("Ready to receive broadcast messages");
}

void loop()
{
    // Update the display if new data received
    if (newDataReceived)
    {
        sevseg.setNumber(marbleCount, 0);
        newDataReceived = false;
    }

    // Must run continuously to keep display updated
    sevseg.refreshDisplay();
}