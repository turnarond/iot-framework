namespace aco::storage {

struct DeviceRecord {
    std::string serial_number;
    std::string public_key_pem;
    std::string status; // "PENDING", "ACTIVE"
    // ...
};

class DeviceRepository {
public:
    static Result<DeviceRecord> get_by_sn(std::string_view sn);
    static Result<void> activate_device(std::string_view sn, X509* cert);
    static Result<void> pre_register(const DeviceRecord& dev);
};

} // namespace aco::storage