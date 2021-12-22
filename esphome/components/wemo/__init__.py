import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components.web_server_base import CONF_WEB_SERVER_BASE_ID

from esphome.components import (
    web_server_base,
)


CONF_DEVICES = "devices"

AUTO_LOAD = ["web_server_base"]
DEPENDENCIES = ["wifi"]



web_server_ns = cg.esphome_ns.namespace("wemo")

WemoManager = web_server_ns.class_(
    "WemoManager", cg.Component,cg.Controller
)

MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        #cv.GenerateID(CONF_WEB_SERVER_BASE_ID): cv.use_id(web_server_base.WebServerBase),        
        cv.GenerateID(): cv.declare_id(WemoManager),
        cv.Optional(CONF_DEVICES): cv.Any(
            "all", cv.ensure_list(cv.use_id(cg.EntityBase))
        ),        
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    #web = await cg.get_variable(config[CONF_WEB_SERVER_BASE_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_DEVICES in config:
        port = 180
        for dev in config[CONF_DEVICES]:
            device = await cg.get_variable(dev)
            cg.add(var.add_device(device,port))
            port=port+1
