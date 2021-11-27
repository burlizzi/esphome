import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CONF_DEVICES = "devices"

AUTO_LOAD = ["web_server_base"]
DEPENDENCIES = ["wifi"]

WemoManager = cg.global_ns.class_(
    "WemoManager", cg.Component
)

MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WemoManager),
        cv.Optional(CONF_DEVICES): cv.Any(
            "all", cv.ensure_list(cv.use_id(cg.EntityBase))
        ),        
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_DEVICES in config:
        for dev in config[CONF_DEVICES]:
            device = await cg.get_variable(dev)
            cg.add(var.add_device(device))
