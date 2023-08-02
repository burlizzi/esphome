
from typing import Literal
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome import automation
from esphome.const import  CONF_ID, CONF_CS_PIN

CONF_HOST = "host"
CONF_PORT = "port"
CONF_KEY = "key"
CONF_DATA_ACTION = "class"

#DEPENDENCIES = ["spi"]


ns = lora_ns = cg.esphome_ns.namespace("serveo")
Lora = ns.class_("Serveo",cg.Component)





#MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Lora),

        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_PORT): cv.int_,
        cv.Required(CONF_KEY): cv.string,


        cv.Optional(CONF_DATA_ACTION): automation.validate_automation(
            single=True
        ),        
        
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    #cg.add_library("https://github.com/burlizzi/Beelan-LoRaWAN", "")
    #cg.add_library("git:../../../Beelan-LoRaWAN", "")
    cg.add_library("LibSSH-ESP32", "")
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.setKey(config[CONF_KEY]))
    cg.add(var.setHost(config[CONF_HOST]))
    cg.add(var.setPort(config[CONF_PORT]))
    
    if CONF_DATA_ACTION in config:
        await automation.build_automation(
            var.data_trigger(), [(int, "port"),(int, "rssi"),(cg.std_string, "data")], config[CONF_DATA_ACTION]
        )
    #cg.add_global(cg.RawStatement(f'const sRFM_pins RFM_pins = {{.CS={config[CONF_CS_PIN]},.RST={config[CONF_RESET]},.DIO0={config[CONF_DIO0]},.DIO1={config[CONF_DIO1]},.DIO2={config[CONF_DIO2]}}};'))
