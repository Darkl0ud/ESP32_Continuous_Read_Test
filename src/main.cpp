#include <Arduino.h>

// Define how many conversions per pin will happen and reading the data will be an average of all adcReadCompletions
#define CONVERSIONS_PER_PIN 1

// Define sample rate 611 - 83,333Hz
#define SAMPLE_RATE 83333

// Declare array of ADC pins that will be used for ADC Continuous mode - ONLY ADC1 pins are supported
// Number of selected pins can be from 1 to ALL ADC1 pins.
#ifdef CONFIG_IDF_TARGET_ESP32
uint8_t adc_pins[] = { 36, 39, 34, 35 };  //some of ADC1 pins for ESP32
#else
uint8_t adc_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};  //ADC1 common pins for ESP32S2/S3 + ESP32C3/C6 + ESP32H2, 1-10 for ESP-S3
#endif

// Calculate how many pins are declared in the array - needed as input for the setup function of ADC Continuous
uint8_t adc_pins_count = sizeof(adc_pins) / sizeof(uint8_t);

// Flag which will be set in ISR when conversion is done
volatile bool adc_conversion_done = false;

// Result structure for ADC Continuous reading
adc_continuous_data_t* result = NULL;

// Stores total amount of conversions
volatile int adcReadCompletions = 0;

// ISR Function that will be triggered when ADC conversion is done
void ARDUINO_ISR_ATTR adcComplete() {
  adc_conversion_done = true;
  adcReadCompletions++;  // Increase total conversions
}

void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogContinuousSetWidth(12);

  // Optional: Set different attenuation (default is ADC_11db)
  analogContinuousSetAtten(ADC_11db);

  // Setup ADC Continuous with following input:
  // array of pins, count of the pins, how many total conversions per pin in one cycle will happen, sampling frequency, callback function
  analogContinuous(adc_pins, adc_pins_count, CONVERSIONS_PER_PIN, SAMPLE_RATE, &adcComplete);

  // Start ADC Continuous conversions
  analogContinuousStart();
}

void loop() {
  // Check if conversion is done and try to read data
  if (adc_conversion_done == true && millis()) {
    // Set ISR flag back to false
    adc_conversion_done = false;

    // Read data from ADC
    if (analogContinuousRead(&result, 0)) {
      
      // Calculate conversions per second based on total conversions and program duration
      float effectiveSampleRate = static_cast<float>((adcReadCompletions  * adc_pins_count * CONVERSIONS_PER_PIN) / (millis() / 1000.0));

      //// Prints conversion rate
      Serial.println("millis(): " + String(millis()) +
                     "  adcReadCompletions: " + String(adcReadCompletions) +
                     "  effectiveSampleRate: " + String(effectiveSampleRate) +
                     "  effectiveSampleRate/Pin: " + String(effectiveSampleRate / adc_pins_count));     
    } else {
      Serial.println("Error occurred during reading data. Set Core Debug Level to error or lower for more information.");
    }
  }
}
