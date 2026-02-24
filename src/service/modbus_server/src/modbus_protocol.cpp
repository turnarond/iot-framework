#include "modbus_protocol.h"
#include <cstring>
#include <arpa/inet.h>

namespace modbus {

// Parse Modbus TCP request
bool parse_modbus_request(const std::vector<uint8_t>& buffer, ModbusRequest& request) {
    if (buffer.size() < 8) {
        return false;
    }

    // Parse TCP header
    memcpy(&request.header, buffer.data(), sizeof(ModbusTCPHeader));
    request.header.transaction_id = ntohs(request.header.transaction_id);
    request.header.protocol_id = ntohs(request.header.protocol_id);
    request.header.length = ntohs(request.header.length);

    // Check protocol ID (should be 0 for Modbus)
    if (request.header.protocol_id != 0) {
        return false;
    }

    // Parse function code
    size_t offset = sizeof(ModbusTCPHeader);
    request.function_code = buffer[offset++];

    // Parse address
    if (buffer.size() >= offset + 2) {
        request.address = (buffer[offset] << 8) | buffer[offset + 1];
        offset += 2;
    }

    // Parse quantity or value
    if (buffer.size() >= offset + 2) {
        request.quantity_or_value = (buffer[offset] << 8) | buffer[offset + 1];
        offset += 2;
    }

    // Parse additional data for write multiple coils/registers
    if (request.function_code == static_cast<uint8_t>(FunctionCode::WRITE_MULTIPLE_COILS) ||
        request.function_code == static_cast<uint8_t>(FunctionCode::WRITE_MULTIPLE_REGISTERS)) {
        if (buffer.size() >= offset + 1) {
            request.byte_count = buffer[offset++];
            if (buffer.size() >= offset + request.byte_count) {
                request.data.assign(buffer.begin() + offset, buffer.begin() + offset + request.byte_count);
            }
        }
    }

    return true;
}

// Build Modbus TCP response
std::vector<uint8_t> build_modbus_response(const ModbusRequest& request, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> response;
    
    // Determine if this is a write operation that doesn't need byte count
    bool is_write_operation = false;
    uint8_t function_code = request.function_code;
    if (function_code == static_cast<uint8_t>(modbus::FunctionCode::WRITE_SINGLE_COIL) ||
        function_code == static_cast<uint8_t>(modbus::FunctionCode::WRITE_SINGLE_REGISTER)) {
        is_write_operation = true;
        response.reserve(sizeof(ModbusTCPHeader) + 1 + data.size());
    } else {
        response.reserve(sizeof(ModbusTCPHeader) + 2 + data.size());
    }

    // Build TCP header
    ModbusTCPHeader header = request.header;
    header.transaction_id = htons(header.transaction_id);
    header.protocol_id = htons(header.protocol_id);
    if (is_write_operation) {
        header.length = htons(1 + 1 + data.size()); // unit_id + function_code + data
    } else {
        header.length = htons(1 + 1 + 1 + data.size()); // unit_id + function_code + byte_count + data
    }
    response.insert(response.end(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(ModbusTCPHeader));

    // Add function code
    response.push_back(request.function_code);

    // Add data
    if (!is_write_operation) {
        // For read operations, add byte count
        response.push_back(data.size());
    }
    response.insert(response.end(), data.begin(), data.end());

    return response;
}

// Build Modbus TCP exception response
std::vector<uint8_t> build_modbus_error_response(const ModbusRequest& request, ExceptionCode exception_code) {
    std::vector<uint8_t> response;
    response.reserve(sizeof(ModbusExceptionResponse));

    // Build TCP header
    ModbusTCPHeader header = request.header;
    header.transaction_id = htons(header.transaction_id);
    header.protocol_id = htons(header.protocol_id);
    header.length = htons(1 + 2); // unit_id + function_code + exception_code
    response.insert(response.end(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(ModbusTCPHeader));

    // Add function code with exception bit set
    response.push_back(request.function_code | 0x80);

    // Add exception code
    response.push_back(static_cast<uint8_t>(exception_code));

    return response;
}

// Calculate CRC (not used in TCP, but included for completeness)
uint16_t calculate_crc(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;

    for (uint8_t byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

} // namespace modbus