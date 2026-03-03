import voluptuous as vol
from homeassistant import config_entries
from homeassistant.core import callback
from homeassistant.const import CONF_HOST, CONF_NAME
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_SCAN_INTERVAL, CONF_PORT, CONF_PREFIX

class Relay16ConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

    @staticmethod
    @callback
    def async_get_options_flow(config_entry):
        return Relay16OptionsFlowHandler(config_entry)

    async def async_step_user(self, user_input=None):
        if user_input is not None:
            return self.async_create_entry(title=user_input[CONF_NAME], data=user_input)

        return self.async_show_form(
            step_id="user",
            data_schema=vol.Schema({
                vol.Required(CONF_HOST): str,
                vol.Required(CONF_PORT, default=30000): int,
                vol.Required(CONF_NAME, default="16 Channel Relay"): str,
                vol.Optional(CONF_PREFIX, default=""): str,
                vol.Optional(CONF_RELAY_COUNT, default=16): int,
                vol.Optional(CONF_SCAN_INTERVAL, default=30): int,
            })
        )

class Relay16OptionsFlowHandler(config_entries.OptionsFlow):

    def __init__(self, config_entry: config_entries.ConfigEntry) -> None:
        super().__init__()

    async def async_step_init(self, user_input=None):
        if user_input is not None:
            return self.async_create_entry(title="", data=user_input)

        options = self.config_entry.options
        data = self.config_entry.data
        host = options.get(CONF_HOST, data.get(CONF_HOST))
        port = options.get(CONF_PORT, data.get(CONF_PORT))
        scan = options.get(CONF_SCAN_INTERVAL, data.get(CONF_SCAN_INTERVAL, 30))

        return self.async_show_form(
            step_id="init",
            data_schema=vol.Schema({
                vol.Required(CONF_HOST, default=host): str,
                vol.Required(CONF_PORT, default=port): int,
                vol.Optional(CONF_SCAN_INTERVAL, default=scan): int,
            }),
        )
