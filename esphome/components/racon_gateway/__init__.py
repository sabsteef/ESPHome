import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome import automation

# Define the component namespace and class
racon_gateway_ns = cg.esphome_ns.namespace('racon_gateway')
RaconGateway = racon_gateway_ns.class_('RaconGateway', cg.Component, uart.UARTDevice)

# Configuration schema without UART_DEVICE_SCHEMA
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RaconGateway),
    cv.Optional("udp_port", default=8125): cv.port,  # Default UDP port
    cv.Required("uart_id"): cv.use_id(uart.UARTComponent),  # Link to UARTComponent
}).extend(cv.COMPONENT_SCHEMA)  # Only extend COMPONENT_SCHEMA

# Define the code generation function
async def to_code(config):
    # Retrieve the UART component instance by ID
    parent = await cg.get_variable(config["uart_id"])

    # Create an instance of RaconGateway with the UART parent
    var = cg.new_Pvariable(config[cv.GenerateID()], parent)

    # Register as a component and associate UART
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    # Set UDP port using the setter
    cg.add(var.set_udp_port(config["udp_port"]))
