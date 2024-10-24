import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
from esphome.components import i2c, time
from esphome.const import CONF_ID
CONF_BATTERY = 'battery1.5v'

## Based on the Official ESPHome DS1307 Driver by badbadc0ffee
CODEOWNERS = ["Trip5"]

DEPENDENCIES = ["i2c"]
rx8025_ns = cg.esphome_ns.namespace("rx8025")
RX8025Component = rx8025_ns.class_("RX8025Component", time.RealTimeClock, i2c.I2CDevice)
WriteAction = rx8025_ns.class_("WriteAction", automation.Action)
ReadAction = rx8025_ns.class_("ReadAction", automation.Action)


CONFIG_SCHEMA = time.TIME_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(RX8025Component),
        cv.Optional(CONF_BATTERY, default=False): cv.boolean,
    }
).extend(i2c.i2c_device_schema(0x64))


@automation.register_action(
    "rx8025.write_time",
    WriteAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(RX8025Component),
        }
    ),
)
async def rx8025_write_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "rx8025.read_time",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(RX8025Component),
        }
    ),
)
async def rx8025_read_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    cg.add(var.set_vdsl(config[CONF_BATTERY]))

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await time.register_time(var, config)

