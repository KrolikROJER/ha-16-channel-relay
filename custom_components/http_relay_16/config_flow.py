import voluptuous as vol
from homeassistant import config_entries
from homeassistant.const import CONF_HOST, CONF_NAME
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_SCAN_INTERVAL, CONF_PORT, CONF_PREFIX

class Relay16ConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    VERSION = 1

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
