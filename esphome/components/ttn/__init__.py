import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import CONF_ID

AUTO_LOAD = ["text_sensor"]
DEPENDENCIES = ["spi"]

ttn_ns = cg.esphome_ns.namespace("ttn")
TTNComponent = ttn_ns.class_("TTNComponent", spi.SPIDevice, cg.Component)

# CONF_VICTRON_ID = "victron_id"   put TTN EUID, APP EUID, APP KEY here

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(spi_device),
        cv.Optional(CONF_BIT_ORDER, default="msb_first"): cv.enum(ORDERS, lower=True),
        cv.Optional(CONF_MODE, default="0"): cv.enum(MODES, upper=True),
    }
).extend(spi.spi_device_schema(False, "1MHz")
).extend( { cv.GenerateID(): cv.declare_id(TTNComponent) } )

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
