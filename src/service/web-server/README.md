# Web Server

Web Server is a RESTful API server built with Oat++ framework for managing edge computing devices, points, alarms, and linkage rules.

## Features

- **Device Management**: Add, update, delete, and retrieve devices and drivers
- **Point Management**: Configure points with addresses, data types, and control settings
- **Real-time Monitoring**: Get current values of points
- **Control Capabilities**: Write values to writable points
- **Alarm Management**: Configure alarm rules with thresholds and methods
- **Linkage Control**: Create rules that trigger actions based on events
- **Comprehensive API**: RESTful endpoints for all management functions

## Prerequisites

- C++17 compatible compiler
- CMake 3.16+
- SQLite3
- Oat++ framework

## Build Instructions

1. **Clone the repository** (if not already done):
   ```bash
   git clone <repository-url>
   cd edge-framework
   ```

2. **Build the web-server**:
   ```bash
   cd src/service/web-server
   mkdir -p build
   cd build
   cmake ..
   cmake --build .
   ```

3. **Run the server**:
   ```bash
   ./web-server
   ```

The server will start on `http://0.0.0.0:8080`.

## API Endpoints

### Devices and Drivers
- `GET /api/drivers` - Get all drivers
- `GET /api/drivers/{id}` - Get driver by ID
- `POST /api/drivers` - Create new driver
- `PUT /api/drivers/{id}` - Update driver
- `DELETE /api/drivers/{id}` - Delete driver

- `GET /api/devices` - Get all devices
- `GET /api/devices/{id}` - Get device by ID
- `POST /api/devices` - Create new device
- `PUT /api/devices/{id}` - Update device
- `DELETE /api/devices/{id}` - Delete device

### Points
- `GET /api/points` - Get all points
- `GET /api/points/{id}` - Get point by ID
- `POST /api/points` - Create new point
- `PUT /api/points/{id}` - Update point
- `DELETE /api/points/{id}` - Delete point

- `GET /api/points/current` - Get current values of all points
- `GET /api/points/value/{name}` - Get current value by point name
- `POST /api/points/{id}/write` - Write value to a point
- `POST /api/points/write/{name}` - Write value to a point by name

### Alarms
- `GET /api/alarm-rules` - Get all alarm rules
- `GET /api/alarm-rules/{id}` - Get alarm rule by ID
- `POST /api/alarm-rules` - Create new alarm rule
- `PUT /api/alarm-rules/{id}` - Update alarm rule
- `DELETE /api/alarm-rules/{id}` - Delete alarm rule

- `GET /api/alarms/active` - Get all active alarms

### Linkage
- `GET /api/event-types` - Get all event types
- `GET /api/trigger-sources` - Get all trigger sources
- `GET /api/action-types` - Get all action types

- `GET /api/action-instances` - Get all action instances
- `POST /api/action-instances` - Create new action instance
- `PUT /api/action-instances/{id}` - Update action instance
- `DELETE /api/action-instances/{id}` - Delete action instance

- `GET /api/linkage-rules` - Get all linkage rules
- `POST /api/linkage-rules` - Create new linkage rule
- `PUT /api/linkage-rules/{id}` - Update linkage rule
- `DELETE /api/linkage-rules/{id}` - Delete linkage rule

- `POST /api/linkage-rules/{ruleId}/triggers/{triggerId}` - Add trigger to linkage rule
- `DELETE /api/linkage-rules/{ruleId}/triggers/{triggerId}` - Remove trigger from linkage rule

- `GET /api/linkage-logs` - Get all linkage logs

## Database

The server uses SQLite3 database with the following main tables:
- `t_drivers` - Driver configurations
- `t_devices` - Device configurations
- `t_points` - Point configurations
- `t_alarm_rules` - Alarm rule definitions
- `t_event_type` - Event type definitions
- `t_trigger_source` - Trigger source configurations
- `t_action_type` - Action type definitions
- `t_action_instance` - Action instance configurations
- `t_linkage_rule` - Linkage rule definitions
- `t_rule_trigger` - Rule-trigger associations
- `t_linkage_log` - Linkage execution logs

The database schema is initialized from `sql/lw_monitor.sql` on first run.

## Configuration

The server runs on port 8080 by default. To change the port, modify the `ConnectionProvider` configuration in `main.cpp`.

## License

Copyright (c) 2026 by ACOINFO. All Rights Reserved.
