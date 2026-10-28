#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "ADC161S626.h"
#include "LMP91000.h"

// Pin definitions (ESP32-S3)
#define UART0_TX_PIN 43
#define UART0_RX_PIN 44
#define SPI0_MOSI_PIN 11
#define SPI0_MISO_PIN 13
#define SPI0_SCLK_PIN 12
#define SPI0_CS_PIN   10
#define I2C0_SCL      9
#define I2C0_SDA      8

// SPI instance (FSPI)
SPIClass spiFSPI(FSPI);

// Create objects
ADC161S626 adc(SPI0_CS_PIN, spiFSPI);
LMP91000 lmp(14);   // MENB pin = GPIO14

// Reference voltages
#define ADC_VREF 2500           // ADC reference in mV (LM4120AIM5-2500)
#define EXTERNAL_REF_MV 2500    // External reference for LMP91000 if used

// Test parameters
//const uint16_t intervals[] = {10, 20, 40, 70, 100, 150, 200};
const uint16_t intervals[] = {200};
const int numIntervals = sizeof(intervals) / sizeof(intervals[0]);

const uint8_t gains[] = {1, 2, 3, 4, 5, 6, 7};   // TIA_GAIN codes
const int numGains = sizeof(gains) / sizeof(gains[0]);

//const uint8_t biases[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}; // bias percent values(1%, 2%, 4%, 6%, 8%, 10%, 12%, 14%, 16%, 18%, 20%, 22%, 24%), default 0=0%
const uint8_t biases[] = {12}; // bias percent values
const int numBiases = sizeof(biases) / sizeof(biases[0]);

const int sampleSize = 200;          // number of samples per test
const int biasApplyIndex = 19;       // apply bias at 20th sample (zero‑based)

// Data storage for current interval: columns = gain * bias, rows = samples
uint16_t data[numGains * numBiases][sampleSize];

// Helper function to delay while printing progress dots every second
void delayWithProgress(unsigned long totalMs) {
  const unsigned long stepMs = 1000; // print a dot every second
  unsigned long elapsed = 0;
  while (elapsed < totalMs) {
    unsigned long wait = min(stepMs, totalMs - elapsed);
    delay(wait);
    elapsed += wait;
    Serial.print(".");
    Serial.flush();   // ensure dot appears immediately
  }
  Serial.print("");   // newline after dots
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // Initialize I2C for LMP91000
  Wire.begin(I2C0_SDA, I2C0_SCL);
  lmp.begin(&Wire, 0x48);
  lmp.setExternalRefMillivolts(EXTERNAL_REF_MV);
  lmp.restoreDefaultsAndLock();   // sets all registers to default (as specified)

  // Unlock registers to allow configuration changes during tests
  lmp.unlockWritableRegisters();

  // Initialize SPI for ADC
  spiFSPI.begin(SPI0_SCLK_PIN, SPI0_MISO_PIN, SPI0_MOSI_PIN, SPI0_CS_PIN);
  adc.setSPIClock(1000000);       // 1 MHz
  adc.begin(ADC_VREF);

  Serial.println("Starting automated test sequence...");
  runTests();
  Serial.println("Test sequence completed.");
}

void loop() {
  // Nothing to do here
}

void runTests() {
  // Loop over all measurement intervals
  for (int iIdx = 0; iIdx < numIntervals; iIdx++) {
    uint16_t interval = intervals[iIdx];
    Serial.print("\n=== Testing interval: ");
    Serial.print(interval);
    Serial.println(" ms ===");

    // For each bias value, test all gains
    for (int bIdx = 0; bIdx < numBiases; bIdx++) {
      uint8_t bias = biases[bIdx];
      Serial.print("\n--- Bias = ");
      Serial.print(bias);
      Serial.println(" ---");

      for (int gIdx = 0; gIdx < numGains; gIdx++) {
        uint8_t gain = gains[gIdx];
        int col = bIdx * numGains + gIdx;   // column index (bias-major order)

        Serial.print("  -> Gain=");
        Serial.print(gain);
        Serial.print(", Bias=");
        Serial.print(bias);
        Serial.print(" ... ");

        // Configure LMP91000 for this test
        lmp.setTIAGain(gain);
        lmp.setBiasSign(LMP91000::LMP91000_bias_negative);   // keep sign negative as per initial config
        lmp.setBiasPercent(0);                                // start with zero bias
        lmp.setOpMode(LMP91000::LMP91000_mode_3lead_amperometric);
        delay(10);   // short settling time after mode change

        // Sampling loop
        for (int s = 0; s < sampleSize; s++) {
          uint16_t raw = adc.readRaw();
          data[col][s] = raw;

          // Apply the test bias at the 20th sample (index 19)
          if (s == biasApplyIndex) {
            lmp.setBiasPercent(bias);
          }

          delay(interval);
        }

        // After sampling, set bias back to zero and allow discharge
        lmp.setBiasPercent(0);
        unsigned long dischargeTime = (unsigned long)interval * sampleSize;
        Serial.print(" discharging");
        delayWithProgress(dischargeTime);   // prints dots during discharge
        Serial.println(" done.");
      }
    }

    // Output CSV for this interval
    printCSVForInterval();
  }
}

void printCSVForInterval() {
  int totalCols = numGains * numBiases;

  // First row: TIA_GAIN for each column
  for (int gIdx = 0; gIdx < numGains; gIdx++) {
    for (int bIdx = 0; bIdx < numBiases; bIdx++) {
      if (gIdx > 0 || bIdx > 0) Serial.print(',');
      Serial.print(gains[gIdx]);
    }
  }
  Serial.println();

  // Second row: BIAS for each column
  for (int gIdx = 0; gIdx < numGains; gIdx++) {
    for (int bIdx = 0; bIdx < numBiases; bIdx++) {
      if (gIdx > 0 || bIdx > 0) Serial.print(',');
      Serial.print(biases[bIdx]);
    }
  }
  Serial.println();

  // Data rows (sample index 0 to sampleSize-1)
  for (int s = 0; s < sampleSize; s++) {
    for (int col = 0; col < totalCols; col++) {
      if (col > 0) Serial.print(',');
      Serial.print(data[col][s]);
    }
    Serial.println();
  }
}