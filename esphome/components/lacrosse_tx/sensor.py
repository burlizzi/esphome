import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import (
    climate,
    remote_receiver,
    sensor,
    remote_base,
)
from esphome.components.remote_base import CONF_RECEIVER_ID

from esphome.const import (
    CONF_DALLAS_ID,
    CONF_INDEX,
    CONF_HUMIDITY,
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    CONF_ID,
    DEVICE_CLASS_HUMIDITY,
    UNIT_PERCENT,
)



AUTO_LOAD = ["sensor", "remote_base"]
CODEOWNERS = ["@luca"]

lacrosse_ns = cg.esphome_ns.namespace("lacrosse")

LacrosseTemperatureSensor = lacrosse_ns.class_("LacrosseSensor", cg.Component)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LacrosseTemperatureSensor),
        cv.GenerateID(CONF_DALLAS_ID): cv.use_id(remote_receiver.RemoteReceiverComponent),
        cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
            unit_of_measurement=UNIT_CELSIUS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_TEMPERATURE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_HUMIDITY): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_HUMIDITY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Required(CONF_INDEX): cv.positive_int,
     }
)



async def to_code(config):
    hub = await cg.get_variable(config[CONF_DALLAS_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_index(config[CONF_INDEX]))
    cg.add(hub.register_listener(var))
    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))
    if CONF_HUMIDITY in config:
        sens = await sensor.new_sensor(config[CONF_HUMIDITY])
        cg.add(var.set_humidity_sensor(sens))
