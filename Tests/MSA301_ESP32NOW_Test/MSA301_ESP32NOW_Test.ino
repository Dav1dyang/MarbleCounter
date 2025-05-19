/*
 * MSA301 Accelerometer + ESP-NOW Test
 * For Adafruit QT Py ESP32 Pico
 *
 * This test detects impacts using the MSA301 accelerometer
 * and broadcasts them via ESP-NOW.
 * Simplified version with no buttons for testing.
 */

#include <Wire.h>
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>
#include <esp_now.h>
#include <WiFi.h>

// Initialize the MSA301 sensor
Adafruit_MSA301 msa;

// LED for visual feedback
const int LED_PIN = LED_BUILTIN;

// Impact detection variables
float threshold = 1.8; // Default threshold in G (medium sensitivity)
unsigned long lastImpactTime = 0;
const unsigned long debounceTime = 300; // Debounce in milliseconds

// Marble counter
uint32_t marbleCount = 0;

// ESP-NOW settings
#define ESPNOW_WIFI_CHANNEL 1

// Callback function for data sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("Data sent successfully");
        // Success flash LED briefly
        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
    }
    else
    {
        Serial.println("Failed to send data");
        // Error flash pattern
        for (int i = 0; i < 2; i++)
        {
            digitalWrite(LED_PIN, HIGH);
            delay(100);
            digitalWrite(LED_PIN, LOW);
            delay(100);
        }
    }
}

// Function to broadcast a marble count
bool broadcast_count(uint32_t count)
{
    char data[32];
    snprintf(data, sizeof(data), "MARBLE:%lu", count);

    Serial.printf("Broadcasting count: %s\n", data);

    // Broadcast address (all FFs)
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Send the message
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)data, sizeof(data));
    return (result == ESP_OK);
}

void setup()
{
    Serial.begin(115200);
    delay(500); // Give serial time to initialize

    Serial.println("MSA301 + ESP-NOW Broadcast Test");

    // Setup LED for visual feedback
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize Wire with the STEMMA QT connector pins
    Wire.begin(SDA1, SCL1);
    Serial.println("I2C initialized");

    // Initialize the MSA301 sensor
    if (!msa.begin())
    {
        Serial.println("Failed to initialize MSA301!");
        while (1)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink to indicate error
            delay(200);
        }
    }
    Serial.println("MSA301 found!");

    // Configure accelerometer settings
    msa.setRange(MSA301_RANGE_2_G);
    msa.setDataRate(MSA301_DATARATE_500_HZ);
    Serial.println("Accelerometer configured");

    // Initialize WiFi for ESP-NOW
    WiFi.mode(WIFI_STA);
    WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
    delay(100); // Give WiFi time to initialize

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
            delay(100);
        }
    }
    Serial.println("ESP-NOW initialized");

    // Register callback function
    esp_now_register_send_cb(OnDataSent);

    // Register broadcast peer
    esp_now_peer_info_t peerInfo = {};
    memset(&peerInfo, 0, sizeof(peerInfo));
    memset(peerInfo.peer_addr, 0xFF, ESP_NOW_ETH_ALEN); // Broadcast address
    peerInfo.channel = ESPNOW_WIFI_CHANNEL;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add broadcast peer");
        while (1)
        {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // Blink to indicate error
            delay(100);
        }
    }
    Serial.println("Broadcast peer added");

    // Initial LED flash to show we're ready
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }

    Serial.printf("Ready to detect marbles! Threshold: %.1f G\n", threshold);
    Serial.println("Each detected marble impact will be broadcast immediately");
}

void loop()
{
    // Read accelerometer data
    sensors_event_t event;
    msa.getEvent(&event);

    // Calculate magnitude of acceleration
    float magnitude = sqrt(event.acceleration.x * event.acceleration.x +
                           event.acceleration.y * event.acceleration.y +
                           event.acceleration.z * event.acceleration.z);

    // Remove gravity component (approximately)
    magnitude = abs(magnitude - 9.8);

    // Detect impact (magnitude above threshold and debounce time passed)
    if (magnitude > threshold && (millis() - lastImpactTime > debounceTime))
    {
        // Update impact time
        lastImpactTime = millis();

        // Increment count
        marbleCount++;

        Serial.printf("Marble detected! Magnitude: %.2f G\n", magnitude);
        Serial.printf("Marble count: %lu\n", marbleCount);

        // Broadcast the updated count
        if (!broadcast_count(marbleCount))
        {
            Serial.println("Failed to broadcast count");
        }
    }

    delay(10); // Short delay between readings
}