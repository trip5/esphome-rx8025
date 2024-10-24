#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace rx8025 {

class RX8025Component : public time::RealTimeClock, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void read_time();
  void write_time();
  void set_vdsl(bool vdsl);

 protected:
  bool read_rtc_();
  bool write_rtc_();
  bool vdsl_{false};
  union rx8025Reg {
    struct {
      // Address 0
      uint8_t second : 4;
      uint8_t second_10 : 3;
      uint8_t unused_0 : 1;
      // Address 1
      uint8_t minute : 4;
      uint8_t minute_10 : 3;
      uint8_t unused_1 : 1;
      // Address 2
      uint8_t hour : 4;
      uint8_t hour_10 : 2;
      uint8_t unused_2 : 2;
      // Address 3
      uint8_t weekday : 3;
      uint8_t unused_3 : 5;
      // Address 4
      uint8_t day : 4;
      uint8_t day_10 : 2;
      uint8_t unused_4 : 2;
      // Address 5
      uint8_t month : 4;
      uint8_t month_10 : 1;
      uint8_t unused_5 : 2;
      uint8_t century_bit : 1;
      //  Address 6
      uint8_t year : 4;
      uint8_t year_10 : 4;
      // Address 7: Digital Offset
      uint8_t precision : 7;
      // TEST is used by the manufacturer for testing. Be sure to write "0" to this bit.
      bool test_bit : 1;
      // Address 8: Alarm_W
      uint8_t alarm_w_min : 4;
      uint8_t alarm_w_min_10 : 3;
      uint8_t unused_8 : 1;
      // Address 9: Alarm_W
      uint8_t alarm_w_hour : 4;
      uint8_t alarm_w_hour_10 : 2;
      uint8_t unused_9 : 2;
      // Address A: Alarm_W
      uint8_t alarm_w_day : 7;
      uint8_t unused_A : 1;
      // Address B: Alarm_D
      uint8_t alarm_d_min : 4;
      uint8_t alarm_d_min_10 : 3;
      uint8_t unused_B : 1;
      // Address C: Alarm_D
      uint8_t alarm_d_hour : 4;
      uint8_t alarm_d_hour_10 : 2;
      uint8_t unused_C : 2;
      // Address D: Reserved
      uint8_t reserved : 8;

      // Below this point almost all bits are documented (for future use?), replaced with unused except for...
      // Address E: Control Register

      // These bits are used to set up the operation of the periodic interrupt function that uses the /INTA pin
      // Combinations of these three bits are used to change the /INTA pin's output status.
      //bool ct0 : 1;
      //bool ct1 : 1;
      //bool ct2 : 1;

      // This bit is used by the manufacturer for testing. Be sure to write "0" to this bit.
      // 0 = Normal operation / 1 = Setting prohibited (manufacturer's test mode)
      //bool manufacturer_test_bit : 1;

      // /CLEN2 combines /CLEN1 bit, and is bit controlling FOUT output.
      // When /CLEN1 and /CLEN2 bit become 1 even if FOE input is "H", FOUT output stops.
      // If FOUT output is unnecessary, it functions as RAM-bit and is ( FOE="L" )
      // When PON bit became 1 because power-on reset function worked, /CLEN1 and /CLEN2 bit become 0.
      //bool clen2_bit : 1;

      uint8_t unused_E1 : 4;

      // /12,24 bit : 0 = 12-hour clock (default) / 1 = 24-hour clock
      bool twelve_twentyfour_bit : 1;

      uint8_t unused_E2 : 2;

      // This bit is used to set up the Alarm D function (to generate alarms matching hour or minute settings).
      //bool dale : 1;

      // This bit is used to set up the Alarm W function (to generate alarms matching day, hour, or minute settings).
      //bool wale : 1;

      // Address F: Control Register 

      // This bit is valid only when the DALE bit value is "1". The DAFG bit value becomes "1" when Alarm D has occurred.
      // The /INTA = "L" status that is set at this time can be set to OFF by writing a "0" to this bit.
      //bool dafg : 1;

      // This bit is valid only when the WALE bit value is "1". The WAFG bit value becomes "1" when Alarm W has occurred.
      // The /INTB = "L" status that is set at this time can be set to OFF by writing a "0" to this bit.
      //bool wafg : 1;

      // During a read operation, this bit indicates the /INTA pin's priodic interrupt output status.
      // This status can be set as OFF by writing a "0" to this bit when /INTA = " L".
      //bool ctfg : 1;

      // This bit is controlling FOUT output with /CLEN2 bit.
      // When /CLEN1 and /CLEN2 bit set to 1 even if FOE input is "H", FOUT output stops.
      // If FOUT output is unnecessary, it functions as RAM-bit and is ( FOE="L" )
      // When PON bit became 1 because power-on reset function worked, /CLEN1 and /CLEN2 bit become 0.
      //bool clen1 : 1;

      uint8_t unused_F : 3;

      // This bit indicates the power-on reset detection function's detection results.
      // The PON bit is set (= 1) when the internal power-on reset function operates.
      bool pon : 1;

      // This bit indicates the oscillation stop detection function's detection results.
      // If a "1" has already been written to this bit, it is cleared to zero when stopping of internal oscillation is detected.
      bool xst : 1;

      // This bit indicates the power drop detection function's detection results.
      // VDET = "1" once a power voltage drop has occurred.
      bool vdet : 1;

      // This bit is used to set the power drop detection function's threshold voltage value.
      // 0 = 2.1V / 1 = 1.3V
      bool vdsl : 1;

    } reg;
    mutable uint8_t raw[sizeof(reg)];
  } rx8025_;
};

template<typename... Ts> class WriteAction : public Action<Ts...>, public Parented<RX8025Component> {
 public:
  void play(Ts... x) override { this->parent_->write_time(); }
};

template<typename... Ts> class ReadAction : public Action<Ts...>, public Parented<RX8025Component> {
 public:
  void play(Ts... x) override { this->parent_->read_time(); }
};
}  // namespace rx8025
}  // namespace esphome
