#include "LMP91000.h"

LMP91000::LMP91000(uint8_t menbPin) : _menbPin(menbPin) {}

void LMP91000::begin(TwoWire* wire, uint8_t i2cAddr) {
  _wire = wire;
  _addr = i2cAddr;
  if (_menbPin != 255) {
    pinMode(_menbPin, OUTPUT);
    digitalWrite(_menbPin, LOW); // MENB 需維持低態進行 I2C 通訊
  }
}

void LMP91000::setMENB(uint8_t menbPin) {
  _menbPin = menbPin;
  pinMode(_menbPin, OUTPUT);
}

void LMP91000::enableMENB() {
  if (_menbPin != 255) {
    digitalWrite(_menbPin, LOW);
  }
}

void LMP91000::disableMENB() {
  if (_menbPin != 255) {
    digitalWrite(_menbPin, HIGH);
  }
}

inline void LMP91000::_beginTransaction() const {
  if (_wire) {
    _wire->beginTransmission(_addr);
  }
}

inline void LMP91000::_endTransaction() const {
  if (_wire) {
    _wire->endTransmission();
  }
}

uint8_t LMP91000::readRegister(uint8_t reg) const {
  if (!_wire) return 0;
  // 寫入目標暫存器位址
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->endTransmission(false);
  // 讀回資料
  _wire->requestFrom((int)_addr, 1);
  if (_wire->available()) {
    return _wire->read();
  }
  return 0;
}

void LMP91000::writeRegister(uint8_t reg, uint8_t value) {
  if (!_wire) return;
  // 若目標為 TIACN 或 REFCN，且 LOCK=1，則禁止寫入並回報錯誤
  if (reg == reg_TIACN || reg == reg_REFCN) {
    uint8_t lock = readLock();
    if ((lock & 0x01) == LMP91000_lock) {
      // 報錯（以 Serial 輸出），並直接返回不寫入
      if (Serial) {
        Serial.println("LMP91000 error: write blocked. LOCK=1 prevents writing TIACN/REFCN");
      }
      return;
    }
  }
  _wire->beginTransmission(_addr);
  _wire->write(reg);
  _wire->write(value);
  _wire->endTransmission();
}

// STATUS
uint8_t LMP91000::readStatus() const { return readRegister(reg_STATUS); }

bool LMP91000::isReady() const {
  return (readStatus() & (1 << locate_STATUS)) == LMP91000_ready; // 0x01 ready
}

// LOCK
uint8_t LMP91000::readLock() const { return readRegister(reg_LOCK); }

void LMP91000::writeLock(uint8_t lockValue) { writeRegister(reg_LOCK, (lockValue & 0x01)); }

// TIACN
uint8_t LMP91000::readTIACN() const { return readRegister(reg_TIACN); }

void LMP91000::setTIAGain(uint8_t gainCode) {
  uint8_t v = readTIACN();
  v &= ~(0b111 << locate_TIA_GAIN);
  v |= ((gainCode & 0b111) << locate_TIA_GAIN);
  writeRegister(reg_TIACN, v);
}

uint8_t LMP91000::getTIAGain() const {
  uint8_t v = readTIACN();
  return (v >> locate_TIA_GAIN) & 0b111;
}

void LMP91000::setRLoad(uint8_t rloadCode) {
  uint8_t v = readTIACN();
  v &= ~(0b11 << locate_RLOAD);
  v |= ((rloadCode & 0b11) << locate_RLOAD);
  writeRegister(reg_TIACN, v);
}

uint8_t LMP91000::getRLoad() const {
  uint8_t v = readTIACN();
  return (v >> locate_RLOAD) & 0b11;
}

// REFCN
uint8_t LMP91000::readREFCN() const { return readRegister(reg_REFCN); }

void LMP91000::setRefSource(uint8_t srcCode) {
  uint8_t v = readREFCN();
  v &= ~(1 << locate_REF_SOURCE);
  v |= ((srcCode & 0x01) << locate_REF_SOURCE);
  writeRegister(reg_REFCN, v);
  // 移除：_currentRefmV 更新，改以 getCurrentRefMillivolts() 計算時動態取得
}

uint8_t LMP91000::getRefSource() const {
  uint8_t v = readREFCN();
  return (v >> locate_REF_SOURCE) & 0x01;
}

void LMP91000::setIntZ(uint8_t intzCode) {
  uint8_t v = readREFCN();
  v &= ~(0b11 << locate_INT_Z);
  v |= ((intzCode & 0b11) << locate_INT_Z);
  writeRegister(reg_REFCN, v);
}

uint8_t LMP91000::getIntZ() const {
  uint8_t v = readREFCN();
  return (v >> locate_INT_Z) & 0b11;
}

void LMP91000::setBiasSign(uint8_t signCode) {
  uint8_t v = readREFCN();
  v &= ~(1 << locate_BIAS_SIGN);
  v |= ((signCode & 0x01) << locate_BIAS_SIGN);
  writeRegister(reg_REFCN, v);
}

uint8_t LMP91000::getBiasSign() const {
  uint8_t v = readREFCN();
  return (v >> locate_BIAS_SIGN) & 0x01;
}

uint8_t LMP91000::getBiasPercent() const {
  uint8_t v = readREFCN();
  return _biasCodeToPercent((v >> locate_BIAS) & 0b1111);
}

void LMP91000::setBiasPercent(uint8_t biasPercent) {
  uint8_t v = readREFCN();
  v &= ~(0b1111 << locate_BIAS);
  v |= ((_biasPercentToCode(biasPercent) & 0b1111) << locate_BIAS);
  writeRegister(reg_REFCN, v);
}

// 轉換：百分比到代碼
uint8_t LMP91000::_biasPercentToCode(uint8_t pct) {
  switch (pct) {
    case 0:  return LMP91000_bias_0pc;
    case 1:  return LMP91000_bias_1pc;
    case 2:  return LMP91000_bias_2pc;
    case 4:  return LMP91000_bias_4pc;
    case 6:  return LMP91000_bias_6pc;
    case 8:  return LMP91000_bias_8pc;
    case 10: return LMP91000_bias_10pc;
    case 12: return LMP91000_bias_12pc;
    case 14: return LMP91000_bias_14pc;
    case 16: return LMP91000_bias_16pc;
    case 18: return LMP91000_bias_18pc;
    case 20: return LMP91000_bias_20pc;
    case 22: return LMP91000_bias_22pc;
    case 24: return LMP91000_bias_24pc;
    default: return LMP91000_bias_0pc; // 不支援的值回退到 0%
  }
}

// 轉換：代碼到百分比
uint8_t LMP91000::_biasCodeToPercent(uint8_t code) {
  switch (code & 0x0F) {
    case LMP91000_bias_0pc:  return 0;
    case LMP91000_bias_1pc:  return 1;
    case LMP91000_bias_2pc:  return 2;
    case LMP91000_bias_4pc:  return 4;
    case LMP91000_bias_6pc:  return 6;
    case LMP91000_bias_8pc:  return 8;
    case LMP91000_bias_10pc: return 10;
    case LMP91000_bias_12pc: return 12;
    case LMP91000_bias_14pc: return 14;
    case LMP91000_bias_16pc: return 16;
    case LMP91000_bias_18pc: return 18;
    case LMP91000_bias_20pc: return 20;
    case LMP91000_bias_22pc: return 22;
    case LMP91000_bias_24pc: return 24;
    default: return 0;
  }
}
// MODECN
uint8_t LMP91000::readMODECN() const { return readRegister(reg_MODECN); }

void LMP91000::setFETShort(uint8_t fetEnable) {
  uint8_t v = readMODECN();
  v &= ~(1 << locate_FET_SHORT);
  v |= ((fetEnable & 0x01) << locate_FET_SHORT);
  writeRegister(reg_MODECN, v);
}

uint8_t LMP91000::getFETShort() const {
  uint8_t v = readMODECN();
  return (v >> locate_FET_SHORT) & 0x01;
}

void LMP91000::setOpMode(uint8_t opModeCode) {
  uint8_t v = readMODECN();
  v &= ~(0b111 << locate_OP_MODE);
  v |= ((opModeCode & 0b111) << locate_OP_MODE);
  writeRegister(reg_MODECN, v);
}

uint8_t LMP91000::getOpMode() const {
  uint8_t v = readMODECN();
  return (v >> locate_OP_MODE) & 0b111;
}

void LMP91000::unlockWritableRegisters() { writeLock(LMP91000_unlock); }

void LMP91000::lockWritableRegisters() { writeLock(LMP91000_lock); }

void LMP91000::restoreDefaultsAndLock() {
  // 解鎖以允許寫入 TIACN/REFCN
  unlockWritableRegisters();

  // TIACN 預設：TIA_GAIN = external，RLOAD = 10Ω
  uint8_t tiacn = 0;
  tiacn |= ((LMP91000_external & 0b111) << locate_TIA_GAIN);
  tiacn |= ((LMP91000_10ohm & 0b11) << locate_RLOAD);
  writeRegister(reg_TIACN, tiacn);

  // REFCN 預設：REF_SOURCE = internal，INT_Z = 50%，BIAS_SIGN = negative，BIAS = 0%
  uint8_t refcn = 0;
  refcn |= ((LMP91000_ref_internal & 0x01) << locate_REF_SOURCE);
  refcn |= ((LMP91000_intz_50 & 0b11) << locate_INT_Z);
  refcn |= ((LMP91000_bias_negative & 0x01) << locate_BIAS_SIGN);
  refcn |= ((LMP91000_bias_0pc & 0b1111) << locate_BIAS);
  writeRegister(reg_REFCN, refcn);

  // MODECN 預設：FET_SHORT = disabled，OP_MODE = deep sleep
  uint8_t modecn = 0;
  modecn |= ((LMP91000_fet_disabled & 0x01) << locate_FET_SHORT);
  modecn |= ((LMP91000_mode_deep_sleep & 0b111) << locate_OP_MODE);
  writeRegister(reg_MODECN, modecn);

  // 寫入完成後鎖定可寫暫存器
  lockWritableRegisters();
}

// 外部參考電壓（mV）設定與讀取
void LMP91000::setExternalRefMillivolts(uint16_t mv) {
  _externalRefmV = mv;
  // 移除：若目前使用外部參考則更新 _currentRefmV，改以 getCurrentRefMillivolts() 動態取得
}

uint16_t LMP91000::getExternalRefMillivolts() const {
  return _externalRefmV;
}

uint16_t LMP91000::getCurrentRefMillivolts() const {
  // 以目前 REFCN 的來源判定
  uint8_t src = ((readREFCN() >> locate_REF_SOURCE) & 0x01);
  return (src == LMP91000_ref_internal) ? 3300 : _externalRefmV;
}

// 工具：根據 INT_Z code 與參考電壓算出 Internal Zero 電壓（mV）
uint16_t LMP91000::_computeIntZMillivolts(uint8_t intzCode, uint16_t refmV) {
  switch (intzCode) {
    case LMP91000_intz_20: return (uint16_t)((refmV * 20) / 100);
    case LMP91000_intz_50: return (uint16_t)((refmV * 50) / 100);
    case LMP91000_intz_67: return (uint16_t)((refmV * 67) / 100);
    default: return 0;
  }
}

// 便捷非靜態版本：自動使用目前參考電壓
uint16_t LMP91000::_computeIntZMillivolts(uint8_t intzCode) const {
  return _computeIntZMillivolts(intzCode, getCurrentRefMillivolts());
}

uint8_t LMP91000::_chooseIntZCodeForTarget(uint16_t targetmV) const {
  uint16_t refmV = getCurrentRefMillivolts();
  return _chooseIntZCodeForTarget(targetmV, refmV);
}

// 指定參考電壓版本：選擇最接近目標電壓的 INT_Z 代碼
uint8_t LMP91000::_chooseIntZCodeForTarget(uint16_t targetmV, uint16_t refmV) {
  uint16_t v20 = _computeIntZMillivolts(LMP91000_intz_20, refmV);
  uint16_t v50 = _computeIntZMillivolts(LMP91000_intz_50, refmV);
  uint16_t v67 = _computeIntZMillivolts(LMP91000_intz_67, refmV);
  uint16_t d20 = (targetmV > v20) ? (targetmV - v20) : (v20 - targetmV);
  uint16_t d50 = (targetmV > v50) ? (targetmV - v50) : (v50 - targetmV);
  uint16_t d67 = (targetmV > v67) ? (targetmV - v67) : (v67 - targetmV);
  uint16_t minD = d20; uint8_t code = LMP91000_intz_20;
  if (d50 < minD) { minD = d50; code = LMP91000_intz_50; }
  if (d67 < minD) { code = LMP91000_intz_67; }
  return code;
}

void LMP91000::setIntZByTargetMillivolts(uint16_t targetmV) {
  _targetIntZmV = targetmV;
  // 使用便捷版本自動取目前參考電壓
  uint8_t code = _chooseIntZCodeForTarget(_targetIntZmV);
  setIntZ(code);
  _currentIntZmV = _computeIntZMillivolts(code);
}

uint16_t LMP91000::getCurrentInternalZeroMillivolts() const {
  // 以便捷版本計算目前 INT_Z 對應電壓
  return _computeIntZMillivolts(getIntZ());
}

uint16_t LMP91000::getTargetInternalZeroMillivolts() const {
  return _targetIntZmV;
}

void LMP91000::setIntZByTarget() {
  setIntZByTargetMillivolts(_targetIntZmV);
}

// 字串轉換輔助（顯示用）
const char* LMP91000::tiaGainToStr(uint8_t c) {
  switch (c) {
    case LMP91000_external:  return "external";
    case LMP91000_2k75ohm:   return "2.75k";
    case LMP91000_3k5ohm:    return "3.5k";
    case LMP91000_7kohm:     return "7k";
    case LMP91000_14kohm:    return "14k";
    case LMP91000_35kohm:    return "35k";
    case LMP91000_120kohm:   return "120k";
    case LMP91000_350kohm:   return "350k";
    default:                 return "unknown";
  }
}

const char* LMP91000::rloadToStr(uint8_t c) {
  switch (c) {
    case LMP91000_10ohm:   return "10Ω";
    case LMP91000_33ohm:   return "33Ω";
    case LMP91000_50ohm:   return "50Ω";
    case LMP91000_100ohm:  return "100Ω";
    default:               return "unknown";
  }
}

const char* LMP91000::refSourceStr(uint8_t c) {
  return (c == LMP91000_ref_external) ? "external" : "internal";
}

const char* LMP91000::intZStr(uint8_t c) {
  switch (c) {
    case LMP91000_intz_20:     return "20%";
    case LMP91000_intz_50:     return "50%";
    case LMP91000_intz_67:     return "67%";
    case LMP91000_intz_bypass: return "bypass";
    default:                   return "unknown";
  }
}

const char* LMP91000::biasSignStr(uint8_t c) {
  return (c == LMP91000_bias_positive) ? "positive" : "negative";
}

const char* LMP91000::opModeStr(uint8_t c) {
  switch (c) {
    case LMP91000_mode_deep_sleep:         return "deep sleep";
    case LMP91000_mode_2lead_ground:       return "2-lead ground";
    case LMP91000_mode_standby:            return "standby";
    case LMP91000_mode_3lead_amperometric: return "3-lead amperometric";
    case LMP91000_mode_temp_tia_off:       return "temperature (TIA OFF)";
    case LMP91000_mode_temp_tia_on:        return "temperature (TIA ON)";
    default:                               return "unknown";
  }
}