#pragma once
#include <Arduino.h>
#include <SPI.h>

// Simple, platform-independent Arduino library for TI ADC161S626 (16-bit SPI ADC)
// - Differential input, two's complement output coding
// - Full-scale differential range: ±VREF (peak-to-peak 2*VREF)
// - SPI MSB-first, default SPI mode: MODE0
//
// Usage:
//   1) Ensure your SPI bus is initialized (SPI.begin(...) or custom SPIClass).
//   2) Create instance: ADC161S626 adc(csPin, SPI);
//   3) Call adc.begin(vref_mV); // e.g. 2500 for 2.5V reference
//   4) Get data: uint16_t raw = adc.readRaw();
//      int32_t mv = adc.rawToMilliVoltsSigned(raw); // for differential two's complement
//      // If using single-ended mapping 0..Vref, use rawToMilliVolts (unipolar)

class ADC161S626 {
public:
  ADC161S626(int csPin,
             SPIClass &spi = SPI,
             uint32_t spiClock = 1000000UL,
             uint8_t spiMode = SPI_MODE0);

  bool begin(uint32_t vref_mV);

  // Read a single 16-bit raw sample from the ADC
  uint16_t readRaw();
  // 讀取完整 24-bit 資料框（3 bytes），以便進行位元對齊與除去前兩位
  uint32_t readFrame24();

  // Convert raw code to millivolts for differential (two's complement)
  // Returns signed millivolts in range [-Vref_mV, +Vref_mV]
  int32_t rawToMilliVoltsSigned(uint16_t raw) const;

  // Convert raw code (0..65535) to millivolts for unipolar mapping (0..Vref)
  // Note: ADC161S626 is differential; this mapping is only for convenience
  uint32_t rawToMilliVolts(uint16_t raw) const;

  void setVref(uint32_t mV);
  void setSPIClock(uint32_t hz);
  void setSPIMode(uint8_t mode);

  uint32_t vrefMilliVolts() const;

private:
  int _csPin;
  SPIClass* _spi;
  uint32_t _vref_mV;
  uint32_t _spiClock;
  uint8_t _spiMode;
  SPISettings _settings;
};