#include "modbus_slave.h"

namespace modbus {

ModbusSlave::ModbusSlave(uint8_t slave_id, 
                         size_t coils_count, 
                         size_t discrete_inputs_count, 
                         size_t holding_registers_count, 
                         size_t input_registers_count)
    : slave_id_(slave_id),
      coils_(coils_count, false),
      discrete_inputs_(discrete_inputs_count, false),
      holding_registers_(holding_registers_count, 0),
      input_registers_(input_registers_count, 0) {
}

std::vector<uint8_t> ModbusSlave::handle_request(const ModbusRequest& request) 
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Handle request: slave_id=%d, function_code=0x%02X, address=%d, quantity_or_value=%d", 
                       slave_id_, request.function_code, request.address, request.quantity_or_value);
    
    switch (static_cast<FunctionCode>(request.function_code)) {
        case FunctionCode::READ_COILS:
            return read_coils(request);
        case FunctionCode::READ_DISCRETE_INPUTS:
            return read_discrete_inputs(request);
        case FunctionCode::READ_HOLDING_REGISTERS:
            return read_holding_registers(request);
        case FunctionCode::READ_INPUT_REGISTERS:
            return read_input_registers(request);
        case FunctionCode::WRITE_SINGLE_COIL:
            return write_single_coil(request);   
        case FunctionCode::WRITE_SINGLE_REGISTER:
            return write_single_register(request);       
        case FunctionCode::WRITE_MULTIPLE_COILS:
            return write_multiple_coils(request);
        case FunctionCode::WRITE_MULTIPLE_REGISTERS:
            return write_multiple_registers(request);  
        default:
            g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid function code: slave_id=%d, function_code=0x%02X", slave_id_, request.function_code);
            return build_modbus_error_response(request, ExceptionCode::ILLEGAL_FUNCTION);
    }
}

std::vector<uint8_t> ModbusSlave::read_coils(const ModbusRequest& request) {
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read coils: slave_id=%d, address=%d, quantity=%d", slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, request.quantity_or_value, coils_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for read coils: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::vector<uint8_t> response_data;
    size_t byte_count = (request.quantity_or_value + 7) / 8;
    response_data.resize(byte_count, 0);

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        if (coils_[request.address + i]) {
            response_data[i / 8] |= (1 << (i % 8));
        }
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read coils completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::read_discrete_inputs(const ModbusRequest& request) {
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read discrete inputs: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, request.quantity_or_value, discrete_inputs_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for read discrete inputs: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::vector<uint8_t> response_data;
    size_t byte_count = (request.quantity_or_value + 7) / 8;
    response_data.resize(byte_count, 0);

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        if (discrete_inputs_[request.address + i]) {
            response_data[i / 8] |= (1 << (i % 8));
        }
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read discrete inputs completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::read_holding_registers(const ModbusRequest &request) 
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read holding registers: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, request.quantity_or_value, holding_registers_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for read holding registers: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::vector<uint8_t> response_data;
    response_data.resize(request.quantity_or_value * 2); 

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        uint16_t value = holding_registers_[request.address + i];
        response_data[i * 2] = (value >> 8) & 0xFF;
        response_data[i * 2 + 1] = value & 0xFF;
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read holding registers completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::read_input_registers(const ModbusRequest& request) 
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read input registers: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, request.quantity_or_value, input_registers_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for read input registers: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::vector<uint8_t> response_data;
    response_data.resize(request.quantity_or_value * 2);

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        uint16_t value = input_registers_[request.address + i];
        response_data[i * 2] = (value >> 8) & 0xFF;
        response_data[i * 2 + 1] = value & 0xFF;
    }

    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Read input registers completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);
    
    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::write_single_coil(const ModbusRequest& request) 
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write single coil: slave_id=%d, address=%d, value=0x%04X", 
        slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, 1, coils_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for write single coil: slave_id=%d, address=%d", 
            slave_id_, request.address);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    if (request.quantity_or_value != 0x0000 && request.quantity_or_value != 0xFF00) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid value for write single coil: slave_id=%d, value=0x%04X", 
            slave_id_, request.quantity_or_value);
        ModbusRequest request;
        request.function_code = static_cast<uint8_t>(FunctionCode::WRITE_SINGLE_COIL);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_VALUE);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    coils_[request.address] = (request.quantity_or_value == 0xFF00);
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write single coil completed: slave_id=%d, address=%d, value=%s", slave_id_, 
        request.address, (request.quantity_or_value == 0xFF00 ? "ON" : "OFF"));

    // Return response with address and value
    std::vector<uint8_t> response_data;
    response_data.push_back((request.address >> 8) & 0xFF);
    response_data.push_back(request.address & 0xFF);
    response_data.push_back((request.quantity_or_value >> 8) & 0xFF);
    response_data.push_back(request.quantity_or_value & 0xFF);

    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::write_single_register(const ModbusRequest& request) {
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write single register: slave_id=%d, address=%d, value=0x%04X", 
        slave_id_, request.address, request.quantity_or_value);
    
    if (!is_valid_address(request.address, 1, holding_registers_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for write single register: slave_id=%d, address=%d", 
            slave_id_, request.address);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    holding_registers_[request.address] = request.quantity_or_value;
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write single register completed: slave_id=%d, address=%d, value=0x%04X", 
        slave_id_, request.address, request.quantity_or_value);

    // Return response with address and value
    std::vector<uint8_t> response_data;
    response_data.push_back((request.address >> 8) & 0xFF);
    response_data.push_back(request.address & 0xFF);
    response_data.push_back((request.quantity_or_value >> 8) & 0xFF);
    response_data.push_back(request.quantity_or_value & 0xFF);

    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::write_multiple_coils(const ModbusRequest& request) {
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write multiple coils: slave_id=%d, address=%d, quantity=%d, data_size=%zu", 
        slave_id_, request.address, request.quantity_or_value, request.data.size());
    
    if (!is_valid_address(request.address, request.quantity_or_value, coils_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for write multiple coils: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        bool value = (request.data[i / 8] & (1 << (i % 8))) != 0;
        coils_[request.address + i] = value;
    }
    
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write multiple coils completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);

    // Return response with address and quantity
    std::vector<uint8_t> response_data;
    response_data.push_back((request.address >> 8) & 0xFF);
    response_data.push_back(request.address & 0xFF);
    response_data.push_back((request.quantity_or_value >> 8) & 0xFF);
    response_data.push_back(request.quantity_or_value & 0xFF);

    return build_modbus_response(request, response_data);
}

std::vector<uint8_t> ModbusSlave::write_multiple_registers(const ModbusRequest& request) 
{
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write multiple registers: slave_id=%d, address=%d, quantity=%d, data_size=%zu", 
        slave_id_, request.address, request.quantity_or_value, request.data.size());
    
    if (!is_valid_address(request.address, request.quantity_or_value, holding_registers_.size())) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid address for write multiple registers: slave_id=%d, address=%d, quantity=%d", 
            slave_id_, request.address, request.quantity_or_value);
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_ADDRESS);
    }

    if (request.data.size() != request.quantity_or_value * 2) {
        g_logger.LogMessage(LW_LOGLEVEL_ERROR, "[ModbusSlave] Invalid data size for write multiple registers: slave_id=%d, expected=%d, got=%zu", 
            slave_id_, request.quantity_or_value * 2, request.data.size());
        return build_modbus_error_response(request, ExceptionCode::ILLEGAL_DATA_VALUE);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    for (uint16_t i = 0; i < request.quantity_or_value; i++) {
        uint16_t value = (request.data[i * 2] << 8) | request.data[i * 2 + 1];
        holding_registers_[request.address + i] = value;
    }
    
    
    g_logger.LogMessage(LW_LOGLEVEL_INFO, "[ModbusSlave] Write multiple registers completed: slave_id=%d, address=%d, quantity=%d", 
        slave_id_, request.address, request.quantity_or_value);

    // Return response with address and quantity
    std::vector<uint8_t> response_data;
    response_data.push_back((request.address >> 8) & 0xFF);
    response_data.push_back(request.address & 0xFF);
    response_data.push_back((request.quantity_or_value >> 8) & 0xFF);
    response_data.push_back(request.quantity_or_value & 0xFF);

    return build_modbus_response(request, response_data);
}

bool ModbusSlave::is_valid_address(uint16_t address, uint16_t quantity, size_t max_count) const {
    return (address < max_count) && (address + quantity <= max_count);
}

} // namespace modbus