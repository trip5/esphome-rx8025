#include "rx8025.h"
#include "esphome/core/log.h"

// Datasheet:
// - https://www.epsondevice.com/crystal/en/products/rtc/rx8025sa.html

namespace esphome {
namespace rx8025 {

static const char *const TAG = "rx8025";

void RX8025Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up rx8025...");
  if (!this->read_rtc_()) {
    this->mark_failed();
  }
}

void RX8025Component::update() { this->read_time(); }

void RX8025Component::dump_config() {
  ESP_LOGCONFIG(TAG, "rx8025:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with rx8025 failed!");
  }
  ESP_LOGCONFIG(TAG, "  Timezone: '%s'", this->timezone_.c_str());
}

float RX8025Component::get_setup_priority() const { return setup_priority::DATA; }

void RX8025Component::read_time() {
  if (!this->read_rtc_()) {
    return;
  }

  if ((rx8025_.reg.pon) || (!rx8025_.reg.xst)) {
    ESP_LOGW(TAG, "RTC halted, not syncing to system clock.");
    return;
  }
  if ((rx8025_.reg.xst && rx8025_.reg.vdet)) {
    ESP_LOGW(TAG, "RTC battery low but continuing sync.");
  }

  ESPTime rtc_time{
      .second = uint8_t(rx8025_.reg.second + 10 * rx8025_.reg.second_10),
      .minute = uint8_t(rx8025_.reg.minute + 10u * rx8025_.reg.minute_10),
      .hour = uint8_t(rx8025_.reg.hour + 10u * rx8025_.reg.hour_10),
      .day_of_week = uint8_t(rx8025_.reg.weekday),
      .day_of_month = uint8_t(rx8025_.reg.day + 10u * rx8025_.reg.day_10),
      .day_of_year = 1,  // ignored by recalc_timestamp_utc(false)
      .month = uint8_t(rx8025_.reg.month + 10u * rx8025_.reg.month_10),
      .year = uint16_t(rx8025_.reg.year + 10u * rx8025_.reg.year_10 + 2000),
      .is_dst = false,  // not used
      .timestamp = 0    // overwritten by recalc_timestamp_utc(false)
  };
  rtc_time.recalc_timestamp_utc(false);
  if (!rtc_time.is_valid()) {
    ESP_LOGE(TAG, "Invalid RTC time, not syncing to system clock.");
    return;
  }
  time::RealTimeClock::synchronize_epoch_(rtc_time.timestamp);
}

void RX8025Component::write_time() {
  auto now = time::RealTimeClock::utcnow();
  if (!now.is_valid()) {
    ESP_LOGE(TAG, "Invalid system time, not syncing to RTC.");
    return;
  }
  rx8025_.reg.year = (now.year - 2000) % 10;
  rx8025_.reg.year_10 = (now.year - 2000) / 10 % 10;
  rx8025_.reg.month = now.month % 10;
  rx8025_.reg.month_10 = now.month / 10;
  rx8025_.reg.day = now.day_of_month % 10;
  rx8025_.reg.day_10 = now.day_of_month / 10;
  rx8025_.reg.weekday = now.day_of_week;
  rx8025_.reg.hour = now.hour % 10;
  rx8025_.reg.hour_10 = now.hour / 10;
  rx8025_.reg.minute = now.minute % 10;
  rx8025_.reg.minute_10 = now.minute / 10;
  rx8025_.reg.second = now.second % 10;
  rx8025_.reg.second_10 = now.second / 10;
  rx8025_.reg.twelve_twentyfour_bit = true;
  rx8025_.reg.pon = false;
  rx8025_.reg.xst = true;
  rx8025_.reg.vdet = false;
  rx8025_.reg.vdsl = this->vdsl_;
  this->write_rtc_();
}

bool RX8025Component::read_rtc_() {
  if (!this->read_bytes(0, this->rx8025_.raw, sizeof(this->rx8025_.raw))) {
    ESP_LOGE(TAG, "Can't read I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Read  %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u [ Checking Registers: 24 Hour: %s vdet: Voltage %s (vdsl: %s Threshold) xst: Oscillator %s pon: Power On Reset Detection %s ]",
            rx8025_.reg.hour_10, rx8025_.reg.hour, rx8025_.reg.minute_10, rx8025_.reg.minute, rx8025_.reg.second_10, rx8025_.reg.second,
            rx8025_.reg.year_10, rx8025_.reg.year, rx8025_.reg.month_10, rx8025_.reg.month, rx8025_.reg.day_10, rx8025_.reg.day,
            ONOFF(rx8025_.reg.twelve_twentyfour_bit), (rx8025_.reg.vdet ? "LOW" : "Normal"), (rx8025_.reg.vdsl ? "1.3V" : "2.1V"),
            (rx8025_.reg.xst ? "Normal" : "STOPPED"), (rx8025_.reg.pon ? "WARNING" : "Normal"));
  return true;
}

bool RX8025Component::write_rtc_() {
  if (!this->write_bytes(0, this->rx8025_.raw, sizeof(this->rx8025_.raw))) {
    ESP_LOGE(TAG, "Can't write I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Write  %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  [ Setting defaults: 24 Hour: %s vdet: Voltage %s (vdsl: %s Threshold) xst: Oscillator %s pon: Power On Reset Detection %s ]",
            rx8025_.reg.hour_10, rx8025_.reg.hour, rx8025_.reg.minute_10, rx8025_.reg.minute, rx8025_.reg.second_10, rx8025_.reg.second,
            rx8025_.reg.year_10, rx8025_.reg.year, rx8025_.reg.month_10, rx8025_.reg.month, rx8025_.reg.day_10, rx8025_.reg.day,
            ONOFF(rx8025_.reg.twelve_twentyfour_bit), (rx8025_.reg.vdet ? "LOW" : "Normal"), (rx8025_.reg.vdsl ? "1.3V" : "2.1V"),
            (rx8025_.reg.xst ? "Normal" : "STOPPED"), (rx8025_.reg.pon ? "WARNING" : "Normal"));
  return true;
}

void RX8025Component::set_vdsl(bool vdsl) { 
    this->vdsl_ = vdsl; 
}
}  // namespace rx8025
}  // namespace esphome
