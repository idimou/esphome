#pragma once

#include "esphome/core/component.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace ttn {

class TTNComponent : public spi::SPIDevice, public Component {
   public:
  void setup() override;
  void dump_config() override;

 protected:
  void write_register(uint8_t reg, uint8_t mask, uint8_t bits, uint8_t start_position) override;
  uint8_t read_register(uint8_t reg) override;
};

}  // namespace ttn
}  // namespace esphome
