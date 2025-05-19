/*
 * Marble Detector
 *
 * Uses Adafruit QT Py ESP32 Pico with MSA301 Accelerometer
 * Detects marble drops via tap detection and sends count via ESP-NOW
 * Simplified version without buttons for stability
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

//--------- CONFIGURABLE SETTINGS - ADJUST THESE AS NEEDED ----------
// Tap detection sensitivity (1-255, higher value = less sensitive)
// 255 is least sensitive, 1 is most sensitive
uint8_t tapThreshold = 200; // Default setting - least sensitive for stability

// Tap duration setting - options: MSA301_TAPDUR_50_MS, MSA301_TAPDUR_100_MS,
// MSA301_TAPDUR_150_MS, MSA301_TAPDUR_200_MS, MSA301_TAPDUR_250_MS, etc.
#define TAP_DURATION MSA301_TAPDUR_250_MS

// Debounce time in milliseconds between detecting taps
const unsigned long TAP_DEBOUNCE_TIME = 500;
//------------------------------------------------------------------

// Tap detection variables
unsigned long lastTapTime = 0;

// Counter variable
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

    Serial.println("Marble Detector - Starting up...");

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
    msa.setPowerMode(MSA301_NORMALMODE);
    msa.setDataRate(MSA301_DATARATE_1000_HZ);
    msa.setBandwidth(MSA301_BANDWIDTH_500_HZ);
    msa.setRange(MSA301_RANGE_2_G);
    msa.setResolution(MSA301_RESOLUTION_14);

    // Configure tap detection
    // Parameters: single tap en, double tap en, duration, threshold
    msa.setClick(true, false, TAP_DURATION, tapThreshold);

    // Enable single tap interrupts (bit 5)
    msa.enableInterrupts(true, false);

    Serial.println("Accelerometer and tap detection configured");
    Serial.printf("Tap threshold: %d (higher = less sensitive)\n", tapThreshold);
    Serial.printf("Debounce time: %lu ms\n", TAP_DEBOUNCE_TIME);

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

    Serial.println("Ready to detect marbles!");

    // Broadcast initial count of 0 on startup
    Serial.println("Broadcasting initial count of 0");
    broadcast_count(0);
    delay(100); // Short delay to ensure message is sent
}

void loop()
{
    // Check for tap detection with debounce
    if (millis() - lastTapTime > TAP_DEBOUNCE_TIME)
    {
        uint8_t motionstat = msa.getMotionInterruptStatus();

        if (motionstat)
        {
            // Check for single tap (bit 5 set)
            if (motionstat & (1 << 5))
            {
                // Update time to prevent multiple triggers
                lastTapTime = millis();

                // Increment count
                marbleCount++;

                Serial.print("Marble detected! (Tap event 0x");
                Serial.print(motionstat, HEX);
                Serial.println(")");
                Serial.printf("Marble count: %lu\n", marbleCount);

                // Broadcast the updated count
                if (!broadcast_count(marbleCount))
                {
                    Serial.println("Failed to broadcast count");
                }

                // Visual feedback of detection
                digitalWrite(LED_PIN, HIGH);
                delay(50);
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    delay(10); // Short delay between readings
}