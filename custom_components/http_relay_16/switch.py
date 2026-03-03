import aiohttp
import logging
import re
from homeassistant.components.switch import SwitchEntity
from homeassistant.const import CONF_HOST, CONF_NAME
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_PORT, CONF_PREFIX

_LOGGER = logging.getLogger(__name__)

async def async_setup_entry(hass, config_entry, async_add_entities):
    host = config_entry.data[CONF_HOST]
    port = config_entry.data[CONF_PORT]
    count = config_entry.data.get(CONF_RELAY_COUNT, 16)


    raw_prefix = config_entry.data.get(CONF_PREFIX, "").strip()
    if not raw_prefix:
        integration_name = config_entry.data.get(CONF_NAME, "relay")
        prefix = integration_name.replace(" ", "_").lower()
    else:
        prefix = raw_prefix

    entities = [RelaySwitch(host, port, i, prefix, count) for i in range(1, count + 1)]
    async_add_entities(entities, True)

class RelaySwitch(SwitchEntity):
    def __init__(self, host, port, channel, prefix, total_count):
        self._host = host
        self._port = port
        self._channel = channel
        self._total_count = total_count
        self._attr_name = f"{prefix.replace('_', ' ').capitalize()} {channel}"
        self._attr_unique_id = f"relay_{prefix}_{channel}"
        self._state = False

    @property
    def is_on(self):
        return self._state

    async def async_turn_on(self, **kwargs):
        index = (self._channel * 2) - 1
        if await self._send_command(index):
            self._state = True
            self.async_write_ha_state()

    async def async_turn_off(self, **kwargs):
        index = (self._channel - 1) * 2
        if await self._send_command(index):
            self._state = False
            self.async_write_ha_state()

    async def _send_command(self, index):
        str_index = str(index).zfill(2)
        url = f"http://{self._host}/{self._port}/{str_index}"
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, timeout=5) as response:
                    return response.status == 200
        except Exception as e:
            _LOGGER.error("Ошибка HTTP запроса для %s: %s", self._attr_name, e)
            return False

    async def async_update(self):
        url = f"http://{self._host}/{self._port}/99"
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, timeout=5) as response:
                    if response.status == 200:
                        html = await response.text()
                        matches = re.findall(r'>({2,})', html)
                        if matches:
                            full_status = max(matches, key=len)
                            status_str = full_status[:self._total_count]
                            if len(status_str) >= self._channel:
                                new_state = status_str[self._channel - 1] == "1"
                                if self._state != new_state:
                                    self._state = new_state
                                    self.async_write_ha_state()
        except Exception as e:
            _LOGGER.debug("Статус недоступен для %s: %s", self._attr_name, e)
