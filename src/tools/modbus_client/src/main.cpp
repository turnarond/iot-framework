#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

// Modbus协议相关定义
namespace modbus {
    // 功能码
    enum class FunctionCode {
        READ_COILS = 1,
        READ_DISCRETE_INPUTS = 2,
        READ_HOLDING_REGISTERS = 3,
        READ_INPUT_REGISTERS = 4,
        WRITE_SINGLE_COIL = 5,
        WRITE_SINGLE_REGISTER = 6,
        WRITE_MULTIPLE_COILS = 15,
        WRITE_MULTIPLE_REGISTERS = 16
    };

    // 异常码
    enum class ExceptionCode {
        ILLEGAL_FUNCTION = 1,
        ILLEGAL_DATA_ADDRESS = 2,
        ILLEGAL_DATA_VALUE = 3,
        SLAVE_DEVICE_FAILURE = 4,
        ACKNOWLEDGE = 5,
        SLAVE_DEVICE_BUSY = 6,
        NEGATIVE_ACKNOWLEDGE = 7,
        MEMORY_PARITY_ERROR = 8
    };

    // Modbus TCP头部
    struct TCPHeader {
        uint16_t transaction_id;
        uint16_t protocol_id;
        uint16_t length;
        uint8_t unit_id;
    } __attribute__((packed));

    // 构建读取请求
    std::vector<uint8_t> build_read_request(uint16_t transaction_id, uint8_t unit_id, FunctionCode function_code, uint16_t address, uint16_t quantity) {
        std::vector<uint8_t> request;
        TCPHeader header = {
            htons(transaction_id),
            htons(0), // protocol_id
            htons(6), // length
            unit_id
        };
        request.insert(request.end(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(TCPHeader));
        request.push_back(static_cast<uint8_t>(function_code));
        request.push_back((address >> 8) & 0xFF);
        request.push_back(address & 0xFF);
        request.push_back((quantity >> 8) & 0xFF);
        request.push_back(quantity & 0xFF);
        return request;
    }

    // 构建写入单个寄存器请求
    std::vector<uint8_t> build_write_request(uint16_t transaction_id, uint8_t unit_id, FunctionCode function_code, uint16_t address, uint16_t value) {
        std::vector<uint8_t> request;
        TCPHeader header = {
            htons(transaction_id),
            htons(0), // protocol_id
            htons(6), // length
            unit_id
        };
        request.insert(request.end(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(TCPHeader));
        request.push_back(static_cast<uint8_t>(function_code));
        request.push_back((address >> 8) & 0xFF);
        request.push_back(address & 0xFF);
        request.push_back((value >> 8) & 0xFF);
        request.push_back(value & 0xFF);
        return request;
    }

    // 解析读取响应
    bool parse_read_response(const std::vector<uint8_t>& response, std::vector<uint16_t>& values) {
        if (response.size() < sizeof(TCPHeader) + 2) {
            return false;
        }

        uint8_t function_code = response[sizeof(TCPHeader)];
        if (function_code & 0x80) {
            uint8_t exception_code = response[sizeof(TCPHeader) + 1];
            std::cerr << "Error: Exception code " << static_cast<int>(exception_code) << std::endl;
            return false;
        }

        uint8_t byte_count = response[sizeof(TCPHeader) + 1];
        if (response.size() != sizeof(TCPHeader) + 2 + byte_count) {
            return false;
        }

        values.clear();
        for (size_t i = 0; i < byte_count; i += 2) {
            uint16_t value = (response[sizeof(TCPHeader) + 2 + i] << 8) | response[sizeof(TCPHeader) + 3 + i];
            values.push_back(value);
        }

        return true;
    }

    // 解析写入响应
    bool parse_write_response(const std::vector<uint8_t>& response, uint16_t& address, uint16_t& value) {
        if (response.size() < sizeof(TCPHeader) + 5) {
            return false;
        }

        uint8_t function_code = response[sizeof(TCPHeader)];
        if (function_code & 0x80) {
            uint8_t exception_code = response[sizeof(TCPHeader) + 1];
            std::cerr << "Error: Exception code " << static_cast<int>(exception_code) << std::endl;
            return false;
        }

        address = (response[sizeof(TCPHeader) + 1] << 8) | response[sizeof(TCPHeader) + 2];
        value = (response[sizeof(TCPHeader) + 3] << 8) | response[sizeof(TCPHeader) + 4];

        return true;
    }
}

// 寄存器类型转换
modbus::FunctionCode get_function_code(const std::string& type, bool is_read) {
    if (is_read) {
        if (type == "coil") return modbus::FunctionCode::READ_COILS;
        if (type == "discrete") return modbus::FunctionCode::READ_DISCRETE_INPUTS;
        if (type == "holding") return modbus::FunctionCode::READ_HOLDING_REGISTERS;
        if (type == "input") return modbus::FunctionCode::READ_INPUT_REGISTERS;
    } else {
        if (type == "coil") return modbus::FunctionCode::WRITE_SINGLE_COIL;
        if (type == "holding") return modbus::FunctionCode::WRITE_SINGLE_REGISTER;
    }
    throw std::invalid_argument("Invalid register type");
}

// 连接到Modbus服务器
int connect_to_server(const std::string& host, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error: Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr) <= 0) {
        std::cerr << "Error: Invalid address" << std::endl;
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error: Failed to connect: " << strerror(errno) << std::endl;
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// 发送请求并接收响应
std::vector<uint8_t> send_request(int sockfd, const std::vector<uint8_t>& request) {
    if (send(sockfd, request.data(), request.size(), 0) < 0) {
        std::cerr << "Error: Failed to send request: " << strerror(errno) << std::endl;
        return {};
    }

    std::vector<uint8_t> response(1024);
    ssize_t bytes_received = recv(sockfd, response.data(), response.size(), 0);
    if (bytes_received < 0) {
        std::cerr << "Error: Failed to receive response: " << strerror(errno) << std::endl;
        return {};
    }

    response.resize(bytes_received);
    return response;
}

// 读取寄存器
void read_registers(const std::string& host, int port, int slave_id, const std::string& type, int address, int number, bool list_mode) {
    int sockfd = connect_to_server(host, port);
    if (sockfd < 0) return;

    try {
        modbus::FunctionCode function_code = get_function_code(type, true);
        std::vector<uint8_t> request = modbus::build_read_request(1, slave_id, function_code, address, number);
        std::vector<uint8_t> response = send_request(sockfd, request);

        if (response.empty()) {
            close(sockfd);
            return;
        }

        std::vector<uint16_t> values;
        if (!modbus::parse_read_response(response, values)) {
            close(sockfd);
            return;
        }

        if (list_mode) {
            // 按行列出批量寄存器值
            std::cout << "Address\tValue" << std::endl;
            std::cout << "-------\t-----" << std::endl;
            for (size_t i = 0; i < values.size(); i++) {
                std::cout << (address + i) << "\t" << values[i] << std::endl;
            }
        } else {
            // 简单列出值
            std::cout << "Read " << values.size() << " registers from " << type << " register starting at address " << address << std::endl;
            std::cout << "Values: ";
            for (size_t i = 0; i < values.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << values[i];
            }
            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    close(sockfd);
}

// 写入寄存器
void write_register(const std::string& host, int port, int slave_id, const std::string& type, int address, int value) {
    int sockfd = connect_to_server(host, port);
    if (sockfd < 0) return;

    try {
        modbus::FunctionCode function_code = get_function_code(type, false);
        std::vector<uint8_t> request = modbus::build_write_request(1, slave_id, function_code, address, value);
        std::vector<uint8_t> response = send_request(sockfd, request);

        if (response.empty()) {
            close(sockfd);
            return;
        }

        uint16_t response_address, response_value;
        if (!modbus::parse_write_response(response, response_address, response_value)) {
            close(sockfd);
            return;
        }

        std::cout << "Successfully wrote value " << response_value << " to " << type << " register at address " << response_address << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    close(sockfd);
}

// 显示帮助信息
void show_help(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  read    Read registers from Modbus server" << std::endl;
    std::cout << "  write   Write value to a register" << std::endl;
    std::cout << "  list    List registers values in table format" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --host       Modbus server IP address (default: 127.0.0.1)" << std::endl;
    std::cout << "  -p, --port       Modbus server port (default: 502)" << std::endl;
    std::cout << "  -s, --slave      Slave ID (default: 1)" << std::endl;
    std::cout << "  -t, --type       Register type (coil, discrete, holding, input)" << std::endl;
    std::cout << "  -a, --address    Starting address" << std::endl;
    std::cout << "  -n, --number     Number of registers to read (for read/list commands)" << std::endl;
    std::cout << "  -v, --value      Value to write (for write command)" << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << prog_name << " read -h 127.0.0.1 -p 502 -s 1 -t holding -a 0 -n 10" << std::endl;
    std::cout << "  " << prog_name << " write -h 127.0.0.1 -p 502 -s 1 -t holding -a 0 -v 1234" << std::endl;
    std::cout << "  " << prog_name << " list -h 127.0.0.1 -p 502 -s 1 -t holding -a 0 -n 10" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        show_help(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    std::string host = "127.0.0.1";
    int port = 502;
    int slave_id = 1;
    std::string type;
    int address = -1;
    int number = -1;
    int value = -1;

    // 解析命令行参数
    int opt;
    struct option long_options[] = {
        {"host", required_argument, nullptr, 'h'},
        {"port", required_argument, nullptr, 'p'},
        {"slave", required_argument, nullptr, 's'},
        {"type", required_argument, nullptr, 't'},
        {"address", required_argument, nullptr, 'a'},
        {"number", required_argument, nullptr, 'n'},
        {"value", required_argument, nullptr, 'v'},
        {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc - 1, argv + 1, "h:p:s:t:a:n:v:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = std::stoi(optarg);
                break;
            case 's':
                slave_id = std::stoi(optarg);
                break;
            case 't':
                type = optarg;
                break;
            case 'a':
                address = std::stoi(optarg);
                break;
            case 'n':
                number = std::stoi(optarg);
                break;
            case 'v':
                value = std::stoi(optarg);
                break;
            default:
                show_help(argv[0]);
                return 1;
        }
    }

    // 验证必要参数
    if (type.empty() || address < 0) {
        std::cerr << "Error: Missing required parameters" << std::endl;
        show_help(argv[0]);
        return 1;
    }

    // 执行命令
    if (command == "read") {
        if (number <= 0) {
            std::cerr << "Error: Number of registers must be positive" << std::endl;
            return 1;
        }
        read_registers(host, port, slave_id, type, address, number, false);
    } else if (command == "write") {
        if (value < 0) {
            std::cerr << "Error: Value must be specified" << std::endl;
            return 1;
        }
        write_register(host, port, slave_id, type, address, value);
    } else if (command == "list") {
        if (number <= 0) {
            std::cerr << "Error: Number of registers must be positive" << std::endl;
            return 1;
        }
        read_registers(host, port, slave_id, type, address, number, true);
    } else {
        std::cerr << "Error: Invalid command" << std::endl;
        show_help(argv[0]);
        return 1;
    }

    return 0;
}
