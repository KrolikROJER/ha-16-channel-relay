import logging
import asyncio
from datetime import timedelta
import async_timeout
import aiohttp

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, UpdateFailed

from .const import DOMAIN, CONF_HOST, CONF_SENSORS_COUNT, CONF_RELAYS_COUNT

_LOGGER = logging.getLogger(__name__)
PLATFORMS = ["sensor", "switch"]

async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Инициализация координатора и платформ."""
    host = entry.data[CONF_HOST]
    sensors_count = entry.data[CONF_SENSORS_COUNT]
    relays_count = entry.data[CONF_RELAYS_COUNT]
    
    session = aiohttp.ClientSession()

    async def async_get_data():
        """Единый фоновый запрос данных с ESP32-S3."""
        data = {"temps": [], "states": []}
        try:
            async with async_timeout.timeout(4):
                # 1. Тянем температуры
                if sensors_count > 0:
                    async with session.get(f"http://{host}/api/temperature") as r:
                        if r.status == 200:
                            json_data = await r.json()
                            data["temps"] = json_data.get("temps", [])

                # 2. Тянем состояния реле
                if relays_count > 0:
                    async with session.get(f"http://{host}/api/relay/states") as r:
                        if r.status == 200:
                            json_data = await r.json()
                            data["states"] = json_data.get("states", [])
                            
            return data
        except Exception as err:
            raise UpdateFailed(f"Ошибка связи с ESP32-S3: {err}")

    # Создаем Координатор обновлений (сканирование раз в 5 секунд)
    coordinator = DataUpdateCoordinator(
        hass,
        _LOGGER,
        name=f"esp32_coordinator_{entry.entry_id}",
        update_method=async_get_data,
        update_interval=timedelta(seconds=5),
    )

    await coordinator.async_config_entry_first_refresh()

    hass.data.setdefault(DOMAIN, {})
    hass.data[DOMAIN][entry.entry_id] = {
        "coordinator": coordinator,
        "session": session,
        "host": host,
        "name": entry.data["name"],
        "sensors_count": sensors_count,
        "relays_count": relays_count
    }

    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)
    return True

async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """Выгрузка интеграции."""
    unload_ok = await hass.config_entries.async_unload_platforms(entry, PLATFORMS)
    if unload_ok:
        data = hass.data[DOMAIN].pop(entry.entry_id)
        await data["session"].close()
    return unload_ok
