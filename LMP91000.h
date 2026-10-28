#pragma once
#include <Arduino.h>
#include <Wire.h>

class LMP91000 {
public:
  // I2C 預設位址
  static const uint8_t DEFAULT_I2C_ADDR = 0x48;

  // 寄存器位址
  static const uint8_t reg_STATUS = 0x00;   // 狀態
  static const uint8_t reg_LOCK   = 0x01;   // 寫保護
  static const uint8_t reg_TIACN  = 0x10;   // TIA 控制
  static const uint8_t reg_REFCN  = 0x11;   // 參考控制
  static const uint8_t reg_MODECN = 0x12;   // 模式控制

  // STATUS 旗標
  static const uint8_t LMP91000_not_ready = 0b00000000;
  static const uint8_t LMP91000_ready     = 0b00000001;
  static const uint8_t locate_STATUS      = 0x00; // 狀態位於 bit0

  // LOCK 旗標
  static const uint8_t LMP91000_unlock    = 0b00000000;
  static const uint8_t LMP91000_lock      = 0b00000001;
  static const uint8_t locate_LOCK        = 0x00; // 鎖定位於 bit0

  // TIACN 欄位位移
  static const uint8_t locate_TIA_GAIN    = 0x02; // bits [4:2]
  static const uint8_t locate_RLOAD       = 0x00; // bits [1:0]

  // TIA_GAIN 代碼（未位移，3-bit）
  static const uint8_t LMP91000_external  = 0b000;
  static const uint8_t LMP91000_2k75ohm   = 0b001;
  static const uint8_t LMP91000_3k5ohm    = 0b010;
  static const uint8_t LMP91000_7kohm     = 0b011;
  static const uint8_t LMP91000_14kohm    = 0b100;
  static const uint8_t LMP91000_35kohm    = 0b101;
  static const uint8_t LMP91000_120kohm   = 0b110;
  static const uint8_t LMP91000_350kohm   = 0b111;

  // RLOAD 代碼（未位移，2-bit）
  static const uint8_t LMP91000_10ohm     = 0b00;
  static const uint8_t LMP91000_33ohm     = 0b01;
  static const uint8_t LMP91000_50ohm     = 0b10;
  static const uint8_t LMP91000_100ohm    = 0b11;

  // REFCN 欄位位移
  static const uint8_t locate_REF_SOURCE  = 0x07; // bit7
  static const uint8_t locate_INT_Z       = 0x05; // bits [6:5]
  static const uint8_t locate_BIAS_SIGN   = 0x04; // bit4
  static const uint8_t locate_BIAS        = 0x00; // bits [3:0]

  // REFCN 代碼（未位移）
  // 參考來源
  static const uint8_t LMP91000_ref_internal = 0b0;
  static const uint8_t LMP91000_ref_external = 0b1;
  // Internal Zero (INT_Z)
  static const uint8_t LMP91000_intz_20      = 0b00;
  static const uint8_t LMP91000_intz_50      = 0b01;
  static const uint8_t LMP91000_intz_67      = 0b10;
  static const uint8_t LMP91000_intz_bypass  = 0b11;
  // BIAS 極性
  static const uint8_t LMP91000_bias_negative = 0b0;
  static const uint8_t LMP91000_bias_positive = 0b1;
  // BIAS 百分比對應之 4-bit 代碼（未位移）
  static const uint8_t LMP91000_bias_0pc   = 0b0000;
  static const uint8_t LMP91000_bias_1pc   = 0b0001;
  static const uint8_t LMP91000_bias_2pc   = 0b0010;
  static const uint8_t LMP91000_bias_4pc   = 0b0011;
  static const uint8_t LMP91000_bias_6pc   = 0b0100;
  static const uint8_t LMP91000_bias_8pc   = 0b0101;
  static const uint8_t LMP91000_bias_10pc  = 0b0110;
  static const uint8_t LMP91000_bias_12pc  = 0b0111;
  static const uint8_t LMP91000_bias_14pc  = 0b1000;
  static const uint8_t LMP91000_bias_16pc  = 0b1001;
  static const uint8_t LMP91000_bias_18pc  = 0b1010;
  static const uint8_t LMP91000_bias_20pc  = 0b1011;
  static const uint8_t LMP91000_bias_22pc  = 0b1100;
  static const uint8_t LMP91000_bias_24pc  = 0b1101;
  // BIAS 百分比（允許值）
  static const uint8_t LMP91000_bias_0   = 0;
  static const uint8_t LMP91000_bias_1   = 1;
  static const uint8_t LMP91000_bias_2   = 2;
  static const uint8_t LMP91000_bias_4   = 4;
  static const uint8_t LMP91000_bias_6   = 6;
  static const uint8_t LMP91000_bias_8   = 8;
  static const uint8_t LMP91000_bias_10  = 10;
  static const uint8_t LMP91000_bias_12  = 12;
  static const uint8_t LMP91000_bias_14  = 14;
  static const uint8_t LMP91000_bias_16  = 16;
  static const uint8_t LMP91000_bias_18  = 18;
  static const uint8_t LMP91000_bias_20  = 20;
  static const uint8_t LMP91000_bias_22  = 22;
  static const uint8_t LMP91000_bias_24  = 24;

  // MODECN 欄位位移
  static const uint8_t locate_FET_SHORT   = 0x07; // bit7
  static const uint8_t locate_OP_MODE     = 0x00; // bits [2:0]

  // MODECN 代碼（未位移）
  static const uint8_t LMP91000_fet_disabled = 0b0;
  static const uint8_t LMP91000_fet_enabled  = 0b1;
  // 操作模式（OP_MODE）
  static const uint8_t LMP91000_mode_deep_sleep        = 0b000;
  static const uint8_t LMP91000_mode_2lead_ground      = 0b001;
  static const uint8_t LMP91000_mode_standby           = 0b010;
  static const uint8_t LMP91000_mode_3lead_amperometric= 0b011;
  static const uint8_t LMP91000_mode_temp_tia_off      = 0b110;
  static const uint8_t LMP91000_mode_temp_tia_on       = 0b111;

  // 建構式
  explicit LMP91000(uint8_t menbPin = 255);

  // 初始化（指派 I2C 物件與位址）
  void begin(TwoWire* wire, uint8_t i2cAddr = DEFAULT_I2C_ADDR);
  void setMENB(uint8_t menbPin);
  void enableMENB();
  void disableMENB();

  // STATUS
  uint8_t readStatus() const;
  bool isReady() const;

  // LOCK
  uint8_t readLock() const;
  void writeLock(uint8_t lockValue);

  // TIACN
  uint8_t readTIACN() const;
  void setTIAGain(uint8_t gainCode);
  uint8_t getTIAGain() const;
  void setRLoad(uint8_t rloadCode);
  uint8_t getRLoad() const;

  // REFCN
  uint8_t readREFCN() const;
  void setRefSource(uint8_t srcCode);
  uint8_t getRefSource() const;
  // 參考電壓：外部參考電壓設定與讀取，以及目前生效的參考電壓（mV）
  void setExternalRefMillivolts(uint16_t mv);
  uint16_t getExternalRefMillivolts() const;
  uint16_t getCurrentRefMillivolts() const;
  // 友善別名（與 setRefSource/getRefSource 相同）：
  inline void setReferenceSource(uint8_t srcCode) { setRefSource(srcCode); }
  inline uint8_t getReferenceSource() const { return getRefSource(); }
  void setIntZ(uint8_t intzCode);
  uint8_t getIntZ() const;
  void setBiasSign(uint8_t signCode);
  uint8_t getBiasSign() const;
  void setBiasPercent(uint8_t biasPercent);
  uint8_t getBiasPercent() const;
  // 新增：以目標內部零電壓（mV）計算最接近權重並設定 INT_Z
  void setTargetInternalZeroMillivolts(uint16_t mv);
  uint16_t getTargetInternalZeroMillivolts() const;
  void setIntZByTargetMillivolts(uint16_t targetmV);
  void setIntZByTarget();
  uint16_t getCurrentInternalZeroMillivolts() const;

  // MODECN
  uint8_t readMODECN() const;
  void setFETShort(uint8_t fetEnable);
  uint8_t getFETShort() const;
  void setOpMode(uint8_t opModeCode);
  uint8_t getOpMode() const;

  // 低階暫存器存取
  uint8_t readRegister(uint8_t reg) const;
  void writeRegister(uint8_t reg, uint8_t value);

  // 寫保護輔助
  void unlockWritableRegisters();
  void lockWritableRegisters();
  void restoreDefaultsAndLock();

  // 文字轉換輔助（僅顯示用）：將代碼轉為人類可讀字串
  static const char* tiaGainToStr(uint8_t c);
  static const char* rloadToStr(uint8_t c);
  static const char* refSourceStr(uint8_t c);
  static const char* intZStr(uint8_t c);
  static const char* biasSignStr(uint8_t c);
  static const char* opModeStr(uint8_t c);

private:
  TwoWire* _wire = nullptr;
  uint8_t  _addr = DEFAULT_I2C_ADDR;
  uint8_t  _menbPin = 255; // 255 表示未使用
  uint16_t _externalRefmV = 3300;   // 外部參考電壓（預設 3300mV）
  // 移除：_currentRefmV，改以 getCurrentRefMillivolts() 計算
  uint16_t _targetIntZmV  = 0;      // 目標 Internal Zero 電壓（mV）
  uint16_t _currentIntZmV = 0;      // 目前 Internal Zero 電壓（mV）

  inline void _beginTransaction() const;
  inline void _endTransaction() const;
  static uint8_t _biasPercentToCode(uint8_t pct);
  static uint8_t _biasCodeToPercent(uint8_t code);
  // 新增：計算工具
  static uint16_t _computeIntZMillivolts(uint8_t intzCode, uint16_t refmV);
  static uint8_t  _chooseIntZCodeForTarget(uint16_t targetmV, uint16_t refmV);
  // 便捷非靜態版本：自動使用目前參考電壓
  uint16_t _computeIntZMillivolts(uint8_t intzCode) const;
  uint8_t  _chooseIntZCodeForTarget(uint16_t targetmV) const;
};