import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome import automation
from esphome.const import CONF_ID, CONF_CS_PIN

CONF_BAND = "band"
CONF_DEVEUI = "deveui"
CONF_APPEUI = "appeui"
CONF_APPKEY = "appkey"
CONF_RST_PIN = "rst_pin"
CONF_DIO0_PIN = "dio0_pin"
CONF_DIO1_PIN = "dio1_pin"
CONF_DIO2_PIN = "dio2_pin"
CONF_DIO5_PIN = "dio5_pin"
CONF_DATA_ACTION = "data_action"
CONF_CLASS = "class"
CONF_FREQUENCY_PLAN = "frequency_plan"


DEPENDENCIES = ["spi"]


ns = lora_ns = cg.esphome_ns.namespace("lora")
Lora = ns.class_("Lora", cg.Component)


LORA_CLASS_ENUM = cg.global_ns.enum("devclass_t")

LORA_CLASS = {
    "CLASS_A": LORA_CLASS_ENUM.CLASS_A,
    "CLASS_C": LORA_CLASS_ENUM.CLASS_C,
}

RX2DR_ENUM = cg.global_ns.enum("dataRates_t")

RX2DR = {
    "SF9": RX2DR_ENUM.SF9BW125,
    "SF12": RX2DR_ENUM.SF12BW125,
}


# MULTI_CONF = True
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Lora),
        cv.Required(CONF_BAND): cv.one_of("EU_868", "AS_923", "US_915", "AU_915"),
        cv.Required(CONF_DEVEUI): cv.string,
        cv.Optional(CONF_APPEUI, "0000000000000000"): cv.string,
        cv.Required(CONF_APPKEY): cv.string,
        cv.Optional(CONF_CLASS, "CLASS_C"): cv.enum(LORA_CLASS),
        cv.Optional(CONF_FREQUENCY_PLAN, "SF12"): cv.enum(RX2DR),
        cv.Optional(CONF_CS_PIN, "SS"): pins.internal_gpio_output_pin_number,
        cv.Optional(CONF_RST_PIN, "RST_LoRa"): pins.internal_gpio_output_pin_number,
        cv.Optional(CONF_DIO0_PIN, "DIO0"): pins.internal_gpio_output_pin_number,
        cv.Optional(CONF_DIO1_PIN, "DIO1"): pins.internal_gpio_input_pin_number,
        cv.Optional(CONF_DIO2_PIN, "DIO2"): pins.internal_gpio_input_pin_number,
        cv.Optional(CONF_DIO5_PIN): pins.internal_gpio_input_pin_number,
        cv.Optional(CONF_DATA_ACTION): automation.validate_automation(single=True),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    # cg.add_library("https://github.com/burlizzi/Beelan-LoRaWAN", "")
    # cg.add_library("git:../../../Beelan-LoRaWAN", "")
    cg.add_library("RadioLib", "")
    cg.add_build_flag("-D" + config[CONF_BAND])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.setDevEUI(config[CONF_DEVEUI]))
    cg.add(var.setAppEUI(config[CONF_APPEUI]))
    cg.add(var.setAppKey(config[CONF_APPKEY]))
    cg.add(var.setDeviceClass(config[CONF_CLASS]))
    cg.add(var.setRX2DR(config[CONF_FREQUENCY_PLAN]))

    if CONF_DATA_ACTION in config:
        await automation.build_automation(
            var.data_trigger(),
            [(int, "port"), (int, "rssi"), (cg.std_string, "data")],
            config[CONF_DATA_ACTION],
        )
    cg.add_global(
        cg.RawStatement(
            f"const sRFM_pins RFM_pins = {{.CS={config[CONF_CS_PIN]},.RST={config[CONF_RST_PIN]},.DIO0={config[CONF_DIO0_PIN]},.DIO1={config[CONF_DIO1_PIN]},.DIO2={config[CONF_DIO2_PIN]}}};"
        )
    )
