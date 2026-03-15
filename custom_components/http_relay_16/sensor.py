from homeassistant.components.sensor import SensorEntity
from homeassistant.helpers.update_coordinator import CoordinatorEntity
from .const import DOMAIN

async def async_setup_entry(hass, config_entry, async_add_entities):
    coordinator = hass.data[DOMAIN][config_entry.entry_id]
    
    async_add_entities([
        RelayDiagnosticsSensor(coordinator, config_entry, "Healthcheck", "healthcheck"),
        RelayDiagnosticsSensor(coordinator, config_entry, "Last Success", "last_success"),
        RelayDiagnosticsSensor(coordinator, config_entry, "Last Error", "last_error")
    ])

class RelayDiagnosticsSensor(CoordinatorEntity, SensorEntity):
    def __init__(self, coordinator, config_entry, name_suffix, field_name):
        super().__init__(coordinator)
        self._field = field_name
        self._attr_name = f"Relay {name_suffix}"
        self._attr_unique_id = f"{config_entry.entry_id}_{field_name}"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, coordinator.host)},
        }
        self._attr_entity_category = "diagnostic"

    @property
    def native_value(self):
        return getattr(self.coordinator, self._field, "Unknown")
