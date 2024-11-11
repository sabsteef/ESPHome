#include "esphome/core/application.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "racon_gateway.h"

namespace esphome {
namespace racon_gateway {

static const auto racon_gateway_config_schema = uart::UARTDevice::UART_DEVICE_SCHEMA;

}  // namespace racon_gateway
}  // namespace esphome
