import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import automation

# Define the racon_gateway namespace
racon_gateway_ns = cg.esphome_ns.namespace('racon_gateway')
RaconGateway = racon_gateway_ns.class_('RaconGateway', cg.Component, uart.UARTDevice)

# Configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RaconGateway),
    cv.Optional('uart_id'): cv.use_id(uart.UARTComponent),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

# Code generation function
async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
