from homeassistant.components.sensor import SensorEntity
from homeassistant.helpers.update_coordinator import CoordinatorEntity
from .const import DOMAIN

async def async_setup_entry(hass, config_entry, async_add_entities):
    coordinator = hass.data[DOMAIN][config_entry.entry_id]

    async_add_entities([
        RelayDiagnosticsSensor(coordinator, "Healthcheck", "healthcheck"),
        RelayDiagnosticsSensor(coordinator, "Last Success", "last_success"),
        RelayDiagnosticsSensor(coordinator, "Last Error", "last_error")
    ])

class RelayDiagnosticsSensor(CoordinatorEntity, SensorEntity):
    def __init__(self, coordinator, name, field):
        super().__init__(coordinator)
        self._field = field
        self._attr_name = f"{coordinator.name} {name}"
        self._attr_unique_id = f"{coordinator.host}_{field}"
        self._attr_device_info = {
            "identifiers": {(DOMAIN, coordinator.host)},
        }

        self._attr_entity_category = "diagnostic"

    @property
    def native_value(self):
        return getattr(self.coordinator, self._field, "Unknown")

    @property
    def extra_state_attributes(self):
        if self._field == "last_error":
            return {"error_time": self.coordinator.last_error_time}
        return None
