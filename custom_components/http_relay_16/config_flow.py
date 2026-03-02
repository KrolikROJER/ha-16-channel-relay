import voluptuous as vol
from homeassistant import config_entries
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_SCAN_INTERVAL

class HttpRelayFlow(config_entries.ConfigFlow, domain=DOMAIN):
    async def async_step_user(self, user_input=None):
        if user_input is not None:
            return self.async_create_entry(title=f"Relay {user_input['host']}", data=user_input)

        return self.async_show_form(
            step_id="user",
            data_schema=vol.Schema({
                vol.Required("host", default="192.168.1.4"): str,
                vol.Required("port", default=30000): int,
                vol.Required(CONF_RELAY_COUNT, default=16): int,
                vol.Required(CONF_SCAN_INTERVAL, default=30): int,
            })
        )
