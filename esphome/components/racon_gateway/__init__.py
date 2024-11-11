import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import automation

# Define the component namespace and class
racon_gateway_ns = cg.esphome_ns.namespace('racon_gateway')
RaconGateway = racon_gateway_ns.class_('RaconGateway', cg.Component, uart.UARTDevice)

# Define the schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RaconGateway),
    cv.Required("uart_id"): cv.use_id(uart.UARTComponent),
}).extend(cv.COMPONENT_SCHEMA)

# Code generation function
async def to_code(config):
    # Retrieve the UART component
    parent = await cg.get_variable(config["uart_id"])

    # Create an instance of RaconGateway with UART as parent
    var = cg.new_Pvariable(config[cv.GenerateID()], parent)

    # Register the component and link the UART
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
