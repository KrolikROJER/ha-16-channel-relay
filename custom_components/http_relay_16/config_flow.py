import voluptuous as vol
from homeassistant import config_entries
import homeassistant.helpers.config_validation as cv

from .const import (
    DOMAIN,
    CONF_HOST,
    CONF_NAME,
    CONF_SENSORS_COUNT,
    CONF_RELAYS_COUNT,
)

class Esp32S3ControllerConfigFlow(config_entries.ConfigFlow, domain=DOMAIN):
    """Настройка интеграции через интерфейс Home Assistant."""
    VERSION = 1

    async def async_step_user(self, user_input=None):
        errors = {}
        if user_input is not None:
            return self.async_create_entry(
                title=user_input[CONF_NAME], 
                data=user_input
            )

        # Безопасная схема валидации полей для UI Home Assistant
        data_schema = vol.Schema({
            vol.Required(CONF_HOST, default="kostya-svet.local"): cv.string,
            vol.Required(CONF_NAME, default="ESP32 Контроллер"): cv.string,
            vol.Required(CONF_SENSORS_COUNT, default=2): vol.All(vol.Coerce(int), vol.Range(min=0, max=10)),
            vol.Required(CONF_RELAYS_COUNT, default=16): vol.All(vol.Coerce(int), vol.Range(min=0, max=16)),
        })

        return self.async_show_form(
            step_id="user", data_schema=data_schema, errors=errors
        )
