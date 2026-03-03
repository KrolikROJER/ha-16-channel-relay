import aiohttp
import logging
import re
from datetime import timedelta
from homeassistant.components.switch import SwitchEntity
from homeassistant.const import CONF_HOST, CONF_NAME
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, CoordinatorEntity
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_PORT, CONF_PREFIX, CONF_SCAN_INTERVAL

_LOGGER = logging.getLogger(__name__)

async def async_setup_entry(hass, config_entry, async_add_entities):
    host = config_entry.options.get(CONF_HOST, config_entry.data.get(CONF_HOST))
    port = config_entry.options.get(CONF_PORT, config_entry.data.get(CONF_PORT))
    scan_interval = config_entry.options.get(CONF_SCAN_INTERVAL, config_entry.data.get(CONF_SCAN_INTERVAL, 30))

    count = config_entry.data.get(CONF_RELAY_COUNT, 16)

    raw_prefix = config_entry.data.get(CONF_PREFIX, "").strip()
    prefix = raw_prefix if raw_prefix else config_entry.data.get(CONF_NAME, "relay").replace(" ", "_").lower()

    coordinator = RelayCoordinator(hass, host, port, count, scan_interval)
    await coordinator.async_config_entry_first_refresh()

    entities = [RelaySwitch(coordinator, i, prefix) for i in range(1, count + 1)]
    async_add_entities(entities)

class RelayCoordinator(DataUpdateCoordinator):
    def __init__(self, hass, host, port, count, scan_interval):
        super().__init__(
            hass, _LOGGER, name=f"{DOMAIN}_{host}",
            update_interval=timedelta(seconds=scan_interval),
        )
        self.host = host
        self.port = port
        self.count = count

    async def _async_update_data(self):
        url = f"http://{self.host}/{self.port}/99"
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, timeout=5) as response:
                    if response.status == 200:
                        html = await response.text()
                        match = re.search(r'>({2,})', html)
                        if match:
                            return match.group(1)[:self.count]
        except Exception as e:
            _LOGGER.debug("Update error for %s: %s", self.host, e)
        return None

class RelaySwitch(CoordinatorEntity, SwitchEntity):
    def __init__(self, coordinator, channel, prefix):
        super().__init__(coordinator)
        self._channel = channel
        self._attr_name = f"{prefix.replace('_', ' ').capitalize()} {channel}"
        self._attr_unique_id = f"relay_{prefix}_{channel}"

        self._attr_device_info = {
            "identifiers": {(DOMAIN, self.coordinator.host)},
            "name": f"Relay Controller ({self.coordinator.host})",
            "manufacturer": "KrolikROJER",
            "model": "HTTP Relay Module",
        }

    @property
    def is_on(self):
        if self.coordinator.data:
            return self.coordinator.data[self._channel - 1] == "1"
        return False

    async def async_turn_on(self, **kwargs):
        index = (self._channel * 2) - 1
        if await self._send_command(index):
            await self.coordinator.async_request_refresh()

    async def async_turn_off(self, **kwargs):
        index = (self._channel - 1) * 2
        if await self._send_command(index):
            await self.coordinator.async_request_refresh()

    async def _send_command(self, index):
        url = f"http://{self.coordinator.host}/{self.coordinator.port}/{str(index).zfill(2)}"
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, timeout=5) as response:
                    return response.status == 200
        except Exception:
            return False
