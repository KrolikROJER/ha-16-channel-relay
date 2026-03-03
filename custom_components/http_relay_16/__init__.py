from homeassistant.const import PLATFORMS
from .const import DOMAIN

PLATFORMS = ["switch"]

async def async_setup_entry(hass, entry):
    """Настройка интеграции."""
    entry.async_on_unload(entry.add_update_listener(update_listener))
    
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    return True

async def update_listener(hass, entry):
    """Перезагрузка при изменении настроек в Options Flow."""
    await hass.config_entries.async_reload(entry.entry_id)

async def async_unload_entry(hass, entry):
    """Выгрузка интеграции."""
    return await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
