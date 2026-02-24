#ifndef MODBUS_PROTOCOL_H
#define MODBUS_PROTOCOL_H

#include <cstdint>
#include <vector>
#include <array>

namespace modbus {

// Modbus function codes
enum class FunctionCode : uint8_t {
    READ_COILS = 0x01,
    READ_DISCRETE_INPUTS = 0x02,
    READ_HOLDING_REGISTERS = 0x03,
    READ_INPUT_REGISTERS = 0x04,
    WRITE_SINGLE_COIL = 0x05,
    WRITE_SINGLE_REGISTER = 0x06,
    WRITE_MULTIPLE_COILS = 0x0F,
    WRITE_MULTIPLE_REGISTERS = 0x10
};

// Modbus exception codes
enum class ExceptionCode : uint8_t {
    ILLEGAL_FUNCTION = 0x01,
    ILLEGAL_DATA_ADDRESS = 0x02,
    ILLEGAL_DATA_VALUE = 0x03,
    SLAVE_DEVICE_FAILURE = 0x04,
    ACKNOWLEDGE = 0x05,
    SLAVE_DEVICE_BUSY = 0x06,
    MEMORY_PARITY_ERROR = 0x08,
    GATEWAY_PATH_UNAVAILABLE = 0x0A,
    GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND = 0x0B
};

// Modbus TCP header structure
struct ModbusTCPHeader {
    uint16_t transaction_id;
    uint16_t protocol_id;
    uint16_t length;
    uint8_t unit_id;
} __attribute__((packed));

// Modbus request structure
struct ModbusRequest {
    ModbusTCPHeader header;
    uint8_t function_code;
    uint16_t address;
    uint16_t quantity_or_value;
    // For write multiple coils/registers
    uint8_t byte_count;
    std::vector<uint8_t> data;
};

// Modbus response structure
struct ModbusResponse {
    ModbusTCPHeader header;
    uint8_t function_code;
    uint8_t byte_count;
    std::vector<uint8_t> data;
};

// Modbus exception response structure
struct ModbusExceptionResponse {
    ModbusTCPHeader header;
    uint8_t function_code;
    uint8_t exception_code;
} __attribute__((packed));

// Parse Modbus TCP request
bool parse_modbus_request(const std::vector<uint8_t>& buffer, ModbusRequest& request);

// Build Modbus TCP response
std::vector<uint8_t> build_modbus_response(const ModbusRequest& request, const std::vector<uint8_t>& data);

// Build Modbus TCP exception response
std::vector<uint8_t> build_modbus_error_response(const ModbusRequest& request, ExceptionCode exception_code);

// Calculate CRC (not used in TCP, but included for completeness)
uint16_t calculate_crc(const std::vector<uint8_t>& data);

} // namespace modbus

#endif // MODBUS_PROTOCOL_H