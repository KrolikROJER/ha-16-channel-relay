from homeassistant.components.sensor import SensorEntity, SensorDeviceClass, SensorStateClass
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN

async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry, async_add_entities: AddEntitiesCallback):
    config = hass.data[DOMAIN][entry.entry_id]
    coordinator = config["coordinator"]
    entities = []
    
    for i in range(config["sensors_count"]):
        entities.append(Esp32S3TempSensor(coordinator, config, i, entry.entry_id))
        
    async_add_entities(entities)

class Esp32S3TempSensor(CoordinatorEntity, SensorEntity):
    """Сущность термометра."""
    def __init__(self, coordinator, config, index, entry_id):
        super().__init__(coordinator)
        self._config = config
        self._index = index
        self._attr_name = f"{config['name']} Температура {index + 1}"
        self._attr_unique_id = f"{entry_id}_temp_{index}"
        self._attr_device_class = SensorDeviceClass.TEMPERATURE
        self._attr_state_class = SensorStateClass.MEASUREMENT
        self._attr_native_unit_of_measurement = "°C"

    @property
    def native_value(self):
        """Берем данные из кеша координатора."""
        temps = self.coordinator.data.get("temps", [])
        if self._index < len(temps):
            val = temps[self._index]
            return float(val) if val is not None else None
        return None
