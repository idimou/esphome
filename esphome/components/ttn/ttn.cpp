#include "ttn.h"
#include "esphome/core/log.h"
#include <algorithm>  // std::min
#include "esphome/core/helpers.h"

namespace esphome {
namespace ttn {

static const char *const TAG = "ttn";

void TTNComponent::setup() {
  ESP_LOGI(TAG, "TTNComponent setup started!");
  this->spi_setup();
  ESP_LOGI(TAG, "SPI setup finished!");
  AS3935Component::setup();
}

void TTNComponent::dump_config() {
  AS3935Component::dump_config();
  LOG_PIN("  CS Pin: ", this->cs_);
}

void TTNComponent::write_register(uint8_t reg, uint8_t mask, uint8_t bits, uint8_t start_pos) {
  uint8_t write_reg = this->read_register(reg);

  write_reg &= (~mask);
  write_reg |= (bits << start_pos);

  this->enable();
  this->write_byte(reg);
  this->write_byte(write_reg);
  this->disable();
}

uint8_t TTNComponent::read_register(uint8_t reg) {
  uint8_t value = 0;
  this->enable();
  this->write_byte(reg | SPI_READ_M);
  value = this->read_byte();
  // According to datsheet, the chip select must be written HIGH, LOW, HIGH
  // to correctly end the READ command.
  this->cs_->digital_write(true);
  this->cs_->digital_write(false);
  this->disable();
  ESP_LOGV(TAG, "read_register_: %d", value);
  return value;
}

void TTNComponent::loop() {
  const uint32_t now = millis();
  if ((state_ > 0) && (now - last_transmission_ >= 200)) {
    // last transmission too long ago. Reset RX index.
    ESP_LOGW(TAG, "Last transmission too long ago");
    state_ = 0;
  }

  if (!available())
    return;

  last_transmission_ = now;
  while (available()) {
    uint8_t c;
    read_byte(&c);
    if (state_ == 0) {
      if (c == '\r' || c == '\n') {
        continue;
      }
      label_.clear();
      value_.clear();
      state_ = 1;
    }
    if (state_ == 1) {
      // Start of a ve.direct hex frame
      if (c == ':') {
        state_ = 3;
        continue;
      }
      if (c == '\t') {
        state_ = 2;
      } else {
        label_.push_back(c);
      }
      continue;
    }
    if (state_ == 2) {
      if (label_ == "Checksum") {
        state_ = 0;
        // The checksum is used as end of frame indicator
        if (now - this->last_publish_ >= this->throttle_) {
          this->last_publish_ = now;
          this->publishing_ = true;
        } else {
          this->publishing_ = false;
        }
        continue;
      }
      if (c == '\r' || c == '\n') {
        if (this->publishing_) {
          handle_value_();
        }
        state_ = 0;
      } else {
        value_.push_back(c);
      }
    }
    // Discard ve.direct hex frame
    if (state_ == 3) {
      if (c == '\r' || c == '\n') {
        state_ = 0;
      }
    }
  }
}

}  // namespace ttn
}  // namespace esphome
