import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import remote_base,spi
from esphome.const import CONF_CARRIER_DUTY_PERCENT, CONF_ID, CONF_PIN, CONF_MODE, CONF_FREQUENCY

binary_ns = cg.esphome_ns.namespace("binary")


AUTO_LOAD = ["remote_base"]
DEPENDENCIES = ["spi"]

remote_transmitter_ns = cg.esphome_ns.namespace("rfm12_transmitter")
RemoteTransmitterComponent = remote_transmitter_ns.class_(
    "RemoteTransmitterComponent", remote_base.RemoteTransmitterBase, cg.Component
)

MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RemoteTransmitterComponent),
        cv.Optional(CONF_MODE): cv.templatable(
            cv.int_range(min=0x9400, max=0x94ff),
        ),
        cv.Optional(CONF_FREQUENCY): cv.templatable(
            cv.float_range(min=430.240, max=439.7575),
        ),
        
    }
).extend(spi.spi_device_schema(cs_pin_required=True))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    if CONF_MODE in config:
        cg.add(var.set_mode(config[CONF_MODE]))
    if CONF_FREQUENCY in config:
        cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    await spi.register_spi_device(var, config)

