/*
 * MSA301 Accelerometer Tap Test
 * For Adafruit QT Py ESP32 Pico
 *
 * This test detects single and double taps using the MSA301 accelerometer
 * connected via the STEMMA QT connector.
 */

#include <Wire.h>
#include <Adafruit_MSA301.h>

// Initialize the MSA301 sensor
Adafruit_MSA301 msa;

// Adjustable sensitivity for tap detection (lower value = more sensitive)
// This can be adjusted between 1-255
uint8_t tapThreshold = 255; // Default sensitivity - can be changed in setup()

// Button for adjusting sensitivity
const int SENSITIVITY_BUTTON_PIN = 5; // Adjust pin as needed
bool lastButtonState = HIGH;
unsigned long lastDebounceTimeButton = 0;
unsigned long debounceDelayButton = 50;

// LED for visual feedback
const int LED_PIN = LED_BUILTIN; // Most ESP32 boards have a built-in LED

// Tap counter
int singleTapCount = 0;
int doubleTapCount = 0;

// Tap detection debounce
unsigned long lastTapTime = 0;
const unsigned long TAP_DEBOUNCE_TIME = 1000; // 1 second debounce between tap detections

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10); // Wait for serial port to open

    Serial.println("MSA301 Tap/Doubletap Test");
    Serial.println("-------------------------");
    Serial.println("Interrupt status explanations:");
    Serial.println("0x20 = Single tap (bit 5 set)");
    Serial.println("0x10 = Double tap (bit 4 set)");
    Serial.println("-------------------------");

    // Setup LED for visual feedback
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Setup button for sensitivity adjustment
    pinMode(SENSITIVITY_BUTTON_PIN, INPUT_PULLUP);

    // Initialize Wire with the STEMMA QT connector pins
    Wire.begin(SDA1, SCL1);

    // Try to initialize the MSA301
    if (!msa.begin())
    {
        Serial.println("Failed to find MSA301 chip");
        while (1)
            delay(10);
    }
    Serial.println("MSA301 found!");

    // Configure the accelerometer for tap detection
    msa.setPowerMode(MSA301_NORMALMODE);
    msa.setDataRate(MSA301_DATARATE_1000_HZ);
    msa.setBandwidth(MSA301_BANDWIDTH_500_HZ);
    msa.setRange(MSA301_RANGE_2_G);
    msa.setResolution(MSA301_RESOLUTION_14);

    // Set tap detection parameters
    // Third parameter is tap duration: MSA301_TAPDUR_50_MS, MSA301_TAPDUR_100_MS,
    // MSA301_TAPDUR_150_MS, MSA301_TAPDUR_200_MS, MSA301_TAPDUR_250_MS, etc.
    // Fourth parameter is tap threshold (1-255): lower = more sensitive
    msa.setClick(false, false, MSA301_TAPDUR_250_MS, tapThreshold);

    // Enable single and double tap interrupts
    msa.enableInterrupts(true, true);

    Serial.print("Default tap threshold (sensitivity): ");
    Serial.println(tapThreshold);
    Serial.println("Lower threshold = more sensitive, Higher threshold = less sensitive");
    Serial.println("Press button to adjust sensitivity.");
    Serial.println("Drop marbles to see tap detection!");
    Serial.println("Debounce time between detections: " + String(TAP_DEBOUNCE_TIME) + "ms");

    // Initial LED flash to show we're ready
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

void loop()
{
    // Check for tap detection with debounce
    if (millis() - lastTapTime > TAP_DEBOUNCE_TIME)
    {
        uint8_t motionstat = msa.getMotionInterruptStatus();

        if (motionstat)
        {
            lastTapTime = millis(); // Reset debounce timer

            Serial.print("Tap detected (0x");
            Serial.print(motionstat, HEX);
            Serial.println(")");

            if (motionstat & (1 << 5))
            {
                singleTapCount++;
                Serial.print("***Single tap #");
                Serial.println(singleTapCount);

                // Flash LED once for single tap
                digitalWrite(LED_PIN, HIGH);
                delay(50);
                digitalWrite(LED_PIN, LOW);
            }

            if (motionstat & (1 << 4))
            {
                doubleTapCount++;
                Serial.print("***Double tap #");
                Serial.println(doubleTapCount);

                // Flash LED twice for double tap
                for (int i = 0; i < 2; i++)
                {
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
                    delay(50);
                }
            }

            Serial.println("");
        }
    }

    // Handle button press to adjust sensitivity
    int reading = digitalRead(SENSITIVITY_BUTTON_PIN);
    if (reading != lastButtonState)
    {
        lastDebounceTimeButton = millis();
    }

    if ((millis() - lastDebounceTimeButton) > debounceDelayButton)
    {
        if (reading != lastButtonState)
        {
            lastButtonState = reading;

            if (lastButtonState == LOW)
            { // Button pressed
                // Adjust tap threshold (sensitivity)
                // Decrease by 5 (making it more sensitive), with wrapping
                if (tapThreshold <= 5)
                {
                    tapThreshold = 150; // Reset to less sensitive
                }
                else
                {
                    tapThreshold -= 10; // Increase sensitivity in bigger steps
                }

                Serial.print("Tap sensitivity threshold adjusted to: ");
                Serial.println(tapThreshold);

                // Update the sensor with new threshold
                msa.setClick(false, false, MSA301_TAPDUR_250_MS, tapThreshold);

                // Visual feedback for sensitivity change
                digitalWrite(LED_PIN, HIGH);
                delay(300);
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    lastButtonState = reading;

    delay(10); // Short delay between readings
}