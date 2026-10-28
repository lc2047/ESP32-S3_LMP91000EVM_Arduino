#include "ADC161S626.h"
#include <Arduino.h>
#include <SPI.h>

// 簡單的實作檔，將介面拆到 .cpp，確保跨平台可編譯

ADC161S626::ADC161S626(int csPin, SPIClass &spi, uint32_t spiClock, uint8_t spiMode)
  : _csPin(csPin), _spi(&spi), _vref_mV(2500), _spiClock(spiClock), _spiMode(spiMode),
    _settings(spiClock, MSBFIRST, spiMode) {}

bool ADC161S626::begin(uint32_t vref_mV) {
  _vref_mV = vref_mV;
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
  return true;
}

// 讀取完整 24-bit frame（3 bytes），MSB first
uint32_t ADC161S626::readFrame24() {
  _spi->beginTransaction(_settings);
  digitalWrite(_csPin, LOW);
  uint8_t b0 = _spi->transfer(0x00); // bits 23..16
  uint8_t b1 = _spi->transfer(0x00); // bits 15..8
  uint8_t b2 = _spi->transfer(0x00); // bits 7..0
  digitalWrite(_csPin, HIGH);
  _spi->endTransaction();
  return (static_cast<uint32_t>(b0) << 16) | (static_cast<uint32_t>(b1) << 8) | static_cast<uint32_t>(b2);
}

// 依據規格：有效 16-bit 位於第 3..18 位，需移除前兩位
uint16_t ADC161S626::readRaw() {
  uint32_t frame24 = readFrame24();
  // 移除前兩位（bit 23,22），保留接續的 16 位（bit 21..6）
  uint32_t shifted = (frame24 & 0x3FFFFF) >> 6; // 取低 22 位再右移 6，得到 16 位
  return static_cast<uint16_t>(shifted & 0xFFFF);
}

// 將 16-bit 兩補數碼轉為有號差分毫伏值（±VREF）
int32_t ADC161S626::rawToMilliVoltsSigned(uint16_t raw) const {
  int16_t signedCode = static_cast<int16_t>(raw); // 兩補數解碼
  return (static_cast<int32_t>(signedCode) * static_cast<int32_t>(_vref_mV)) / 32768L;
}

uint32_t ADC161S626::rawToMilliVolts(uint16_t raw) const {
  return (static_cast<uint32_t>(raw) * _vref_mV) / 65535UL;
}

void ADC161S626::setVref(uint32_t mV) { _vref_mV = mV; }

void ADC161S626::setSPIClock(uint32_t hz) {
  _spiClock = hz;
  _settings = SPISettings(_spiClock, MSBFIRST, _spiMode);
}

void ADC161S626::setSPIMode(uint8_t mode) {
  _spiMode = mode;
  _settings = SPISettings(_spiClock, MSBFIRST, _spiMode);
}

uint32_t ADC161S626::vrefMilliVolts() const { return _vref_mV; }