import aiohttp
from homeassistant.components.switch import SwitchEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN

async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback):
    config = hass.data[DOMAIN][entry.entry_id]
    coordinator = config["coordinator"]
    entities = []

    # 1. Поканальные реле
    for i in range(config["relays_count"]):
        entities.append(Esp32S3RelaySwitch(coordinator, config, i, entry.entry_id))

    # 2. Кнопки группового управления
    if config["relays_count"] > 0:
        entities.append(Esp32S3MassActionSwitch(config, "all-on", "Включить все реле", entry.entry_id))
        entities.append(Esp32S3MassActionSwitch(config, "all-off", "Выключить все реле", entry.entry_id))

    async_add_entities(entities)

class Esp32S3RelaySwitch(CoordinatorEntity, SwitchEntity):
    """Поканальный выключатель реле."""
    def __init__(self, coordinator, config, index, entry_id):
        super().__init__(coordinator)
        self._config = config
        self._index = index
        self._attr_name = f"{config['name']} Реле {index + 1}"
        self._attr_unique_id = f"{entry_id}_relay_{index}"

    @property
    def is_on(self):
        states = self.coordinator.data.get("states", [])
        if self._index < len(states):
            return states[self._index]
        return False

    async def async_turn_on(self, **kwargs):
        await self._send_toggle()

    async def async_turn_off(self, **kwargs):
        await self._send_toggle()

    async def _send_toggle(self):
        url = f"http://{self._config['host']}/api/relay/toggle"
        try:
            async with self._config["session"].post(url, data={"id": self._index}, timeout=4) as r:
                if r.status == 200:
                    # Опережающее обновление состояния до следующего пула координатора
                    text = await r.text()
                    states = self.coordinator.data.get("states", [])
                    if self._index < len(states):
                        states[self._index] = (text == "1")
                    self.async_write_ha_state()
        except Exception:
            pass

class Esp32S3MassActionSwitch(SwitchEntity):
    """Кнопки массового действия."""
    def __init__(self, config, action, name, entry_id):
        self._config = config
        self._action = action
        self._attr_name = f"{config['name']} {name}"
        self._attr_unique_id = f"{entry_id}_mass_{action}"
        self._attr_icon = "mdi:flash"

    @property
    def is_on(self):
        return False

    async def async_turn_on(self, **kwargs):
        url = f"http://{self._config['host']}/api/relay/{self._action}"
        try:
            await self._config["session"].post(url, timeout=4)
        except Exception:
            pass
        self.async_write_ha_state()

    async def async_turn_off(self, **kwargs):
        pass
