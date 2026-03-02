import requests
import logging
from datetime import timedelta
from homeassistant.components.switch import SwitchEntity
from .const import DOMAIN

_LOGGER = logging.getLogger(__name__)

async def async_setup_entry(hass, entry, async_add_entities):
    config = entry.data
    entities = []
    for i in range(config["relay_count"]):
        entities.append(HttpRelaySwitch(config, i))
    async_add_entities(entities, update_before_add=True)

class HttpRelaySwitch(SwitchEntity):
    def __init__(self, config, index):
        self._host = config["host"]
        self._port = config["port"]
        self._index = index
        self._attr_name = f"Relay {self._host} P{index + 1}"
        self._attr_unique_id = f"relay_{self._host}_{index}"
        self._state = None

    @property
    def is_on(self):
        return self._state

    def update(self):
        try:
            url = f"http://{self._host}:{self._port}/99"
            response = requests.get(url, timeout=5)
            status_str = response.text.strip()
            if len(status_str) >= self._index + 1:
                self._state = status_str[self._index] == "1"
        except Exception as e:
            _LOGGER.error("Error updating relay status: %s", e)

    def turn_on(self, **kwargs):
        cmd = str(self._index * 2 + 1).zfill(2)
        requests.get(f"http://{self._host}:{self._port}/{cmd}", timeout=5)
        self._state = True

    def turn_off(self, **kwargs):
        cmd = str(self._index * 2).zfill(2)
        requests.get(f"http://{self._host}:{self._port}/{cmd}", timeout=5)
        self._state = False
