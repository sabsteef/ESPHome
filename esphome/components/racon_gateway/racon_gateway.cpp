#include "racon_gateway.h"
#include "esphome/core/log.h"

namespace esphome {
namespace racon_gateway {

static const char *TAG = "racon_gateway";

void RaconGateway::setup() {
  ESP_LOGI(TAG, "Racon Gateway component setup initialized");
}

void RaconGateway::loop() {
  // Log a message to indicate the loop is running
  ESP_LOGI(TAG, "Racon Gateway loop running");
  delay(5000);  // 5-second delay
}

}  // namespace racon_gateway
}  // namespace esphome
