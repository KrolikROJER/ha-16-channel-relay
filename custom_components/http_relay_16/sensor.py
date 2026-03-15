import logging
from homeassistant.components.sensor import SensorEntity
from homeassistant.helpers.update_coordinator import CoordinatorEntity
from .const import DOMAIN

_LOGGER = logging.getLogger(__name__)

async def async_setup_entry(hass, config_entry, async_add_entities):
    coordinator = hass.data[DOMAIN][config_entry.entry_id]

    async_add_entities([
        RelayDiagnosticsSensor(coordinator, "Healthcheck", "healthcheck"),
        RelayDiagnosticsSensor(coordinator, "Last Success", "last_success"),
        RelayDiagnosticsSensor(coordinator, "Last Error", "last_error")
    ])

class RelayDiagnosticsSensor(CoordinatorEntity, SensorEntity):
    def __init__(self, coordinator, name_suffix, field_name):
        super().__init__(coordinator)
        self._field = field_name
        self._attr_name = f"Relay {name_suffix}" # Имя на карточке
        self._attr_unique_id = f"{coordinator.host}_{field_name}_diag"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, coordinator.host)},
        }
        self._attr_entity_category = "diagnostic"

    @property
    def native_value(self):
        val = getattr(self.coordinator, self._field, "Unknown")
        return val if val is not None else "Unknown"
