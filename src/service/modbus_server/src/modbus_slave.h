#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

#include <vector>
#include <mutex>
#include <cstdint>
#include "modbus_protocol.h"
#include <lwlog/lwlog.h>

extern CLWLog g_logger;

namespace modbus {

class ModbusSlave {
public:
    ModbusSlave(uint8_t slave_id, 
                size_t coils_count, 
                size_t discrete_inputs_count, 
                size_t holding_registers_count, 
                size_t input_registers_count);

    // Handle Modbus request
    std::vector<uint8_t> handle_request(const ModbusRequest& request);

    // Get slave ID
    uint8_t get_slave_id() const { return slave_id_; }

private:
    // Read operations
    std::vector<uint8_t> read_coils(const ModbusRequest& request);
    std::vector<uint8_t> read_discrete_inputs(const ModbusRequest& request);
    std::vector<uint8_t> read_holding_registers(const ModbusRequest& request);
    std::vector<uint8_t> read_input_registers(const ModbusRequest& request);

    // Write operations
    std::vector<uint8_t> write_single_coil(const ModbusRequest& request);
    std::vector<uint8_t> write_single_register(const ModbusRequest& request);
    std::vector<uint8_t> write_multiple_coils(const ModbusRequest& request);
    std::vector<uint8_t> write_multiple_registers(const ModbusRequest& request);

    // Check address validity
    bool is_valid_address(uint16_t address, uint16_t quantity, size_t max_count) const;

    // Data storage
    uint8_t slave_id_;
    std::vector<bool> coils_;
    std::vector<bool> discrete_inputs_;
    std::vector<uint16_t> holding_registers_;
    std::vector<uint16_t> input_registers_;

    // Mutex for thread safety
    mutable std::mutex mutex_;
};

} // namespace modbus

#endif // MODBUS_SLAVE_H