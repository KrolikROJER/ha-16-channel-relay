import aiohttp
import logging
import re
import asyncio
from datetime import datetime, timedelta

from homeassistant.components.switch import SwitchEntity
from homeassistant.helpers.update_coordinator import DataUpdateCoordinator, CoordinatorEntity
from homeassistant.helpers import entity_registry as er
from .const import DOMAIN, CONF_RELAY_COUNT, CONF_PORT, CONF_PREFIX, CONF_SCAN_INTERVAL
from homeassistant.const import CONF_HOST, CONF_NAME

_LOGGER = logging.getLogger(__name__)

def retry_api(func):
    async def wrapper(self, *args, **kwargs):
        delays = [0.5, 1.0, 3.0]
        last_ex = None
        
        if func.__name__ == "send_command":
            idx = args[0]
            ch_num = (idx // 2) + 1 if idx % 2 == 0 else (idx + 1) // 2
            context = f"для канала {ch_num}"
            target_url = f"http://{self.host}/{self.port}/{str(idx).zfill(2)}"
        else:
            context = "для запроса статуса"
            target_url = f"http://{self.host}/{self.port}/99"

        for i, delay in enumerate(delays):
            try:
                result = await func(self, *args, **kwargs)
                self.healthcheck = "OK"
                self.last_success = datetime.now().strftime("%d.%m.%Y %H:%M:%S")
                return result
            except Exception as e:
                last_ex = e
                if i == len(delays) - 1:
                    self.healthcheck = "Error"
                    self.last_error = f"Ошибка выполнения ({target_url}) {context}: {str(e)}"
                    self.last_error_time = datetime.now().strftime("%d.%m.%Y %H:%M:%S")
                    _LOGGER.error(self.last_error)
                else:
                    await asyncio.sleep(delay)
        return None
    return wrapper

async def async_setup_entry(hass, config_entry, async_add_entities):
    coordinator = hass.data[DOMAIN][config_entry.entry_id]

    count = config_entry.data.get(CONF_RELAY_COUNT, 16)
    raw_prefix = config_entry.data.get(CONF_PREFIX, "").strip()
    prefix = raw_prefix if raw_prefix else config_entry.data.get(CONF_NAME, "relay").replace(" ", "_").lower()

    entities = [RelaySwitch(coordinator, i, prefix) for i in range(1, count + 1)]
    async_add_entities(entities)

    if not hass.services.has_service(DOMAIN, "turn_all_on"):
        async def handle_mass_control(call):
            registry = er.async_get(hass)
            target_entities = call.data.get("entity_id", [])
            entry_ids = set()
            for eid in target_entities:
                if entry := registry.async_get(eid):
                    entry_ids.add(entry.config_entry_id)

            for e_id in entry_ids:
                if coord := hass.data.get(DOMAIN, {}).get(e_id):
                    await coord.turn_all(call.service == "turn_all_on")

        hass.services.async_register(DOMAIN, "turn_all_on", handle_mass_control)
        hass.services.async_register(DOMAIN, "turn_all_off", handle_mass_control)

class RelayCoordinator(DataUpdateCoordinator):
    def __init__(self, hass, host, port, count, scan_interval):
        super().__init__(
            hass, _LOGGER, name=f"{DOMAIN}_{host}",
            update_interval=timedelta(seconds=scan_interval),
        )
        self.host, self.port, self.count = host, port, count
        self.healthcheck, self.last_success = "Unknown", "Never"
        self.last_error, self.last_error_time = "None", "Never"

    @retry_api
    async def _async_update_data(self):
        url = f"http://{self.host}/{self.port}/99"
        async with aiohttp.ClientSession() as session:
            async with session.get(url, timeout=5) as response:
                response.raise_for_status()
                html = await response.text()
                clean_html = html.replace('\n', '').replace('\r', '').strip()
                match = re.search(r'>({' + str(self.count) + r',})', clean_html)
                if not match: match = re.search(r'>(+)', clean_html)
                if match: return match.group(1)[:self.count]
                raise Exception("Status pattern not found")

    @retry_api
    async def send_command(self, idx):
        url = f"http://{self.host}/{self.port}/{str(idx).zfill(2)}"
        async with aiohttp.ClientSession() as session:
            async with session.get(url, timeout=5) as response:
                response.raise_for_status()
                return True

    async def turn_all(self, state: bool):
        if not self.data: return
        target = "1" if state else "0"
        for i in range(1, self.count + 1):
            if self.data[i-1] != target:
                idx = (i * 2) - 1 if state else (i - 1) * 2
                await self.send_command(idx)
                await asyncio.sleep(0.15) 
        await self.async_request_refresh()

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
        if self.coordinator.data and len(self.coordinator.data) >= self._channel:
            return self.coordinator.data[self._channel - 1] == "1"

        return False

    async def async_turn_on(self, **kwargs):
        if await self.coordinator.send_command((self._channel * 2) - 1):
            await asyncio.sleep(0.5)
            await self.coordinator.async_request_refresh()

    async def async_turn_off(self, **kwargs):
        if await self.coordinator.send_command((self._channel - 1) * 2):
            await asyncio.sleep(0.5)
            await self.coordinator.async_request_refresh()
