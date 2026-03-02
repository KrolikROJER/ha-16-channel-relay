# HTTP Multi-Channel Relay for Home Assistant

[English](#english) | [Русский](#русский)

---

<a name="english"></a>
## English Description
Custom integration for Ethernet/Web relay boards (4, 8, or 16 channels) controlled via HTTP GET commands.

![Relay Board](device_image.jpg)

### Features
- **Plug & Play**: Easy setup via the Home Assistant UI (Config Flow).
- **Dynamic Configuration**: Supports 4, 8, or 16-port devices.
- **Status Sync**: Real-time feedback using the `/99` status command.

### Installation
1. Open **HACS** -> Custom repositories.
2. Add URL: `https://github.com`
3. Category: **Integration**.
4. Install "HTTP Multi-Channel Relay" and **restart HA**.

---

<a name="русский"></a>
## Русский (Russian)
Интеграция для управления релейными платами Ethernet/Web (на 4, 8 или 16 каналов) через HTTP GET команды.

### Основные возможности
- **Настройка через интерфейс**: Никакого редактирования YAML. Просто введите IP и порт.
- **Гибкость**: Поддержка устройств с разным количеством портов (4, 8, 16).
- **Обратная связь**: Чтение текущего состояния реле через команду `/99`.
- **Родные сущности**: Каждый канал появляется как стандартный `switch`, готовый для групп и автоматизаций.

### Установка
1. Откройте **HACS** -> Интеграции.
2. Нажмите на три точки в углу -> **Пользовательские репозитории**.
3. Вставьте ссылку: `https://github.com`
4. Выберите категорию **Интеграция** и нажмите **Добавить**.
5. Найдите её в списке, установите и **перезагрузите Home Assistant**.

### Настройка (Config Flow)
1. Зайдите в **Настройки** -> **Устройства и службы**.
2. Нажмите **Добавить интеграцию**.
3. Найдите **HTTP Multi-Channel Relay**.
4. Введите IP (по умолчанию `192.168.1.4`), порт (`30000`) и выберите количество каналов.

### API Команды
- **Включить**: `http://<ip>:<port>/01`, `03`, `05`...
- **Выключить**: `http://<ip>:<port>/00`, `02`, `04`...
- **Статус**: `http://<ip>:<port>/99` (возвращает строку из 16 символов ASCII).
