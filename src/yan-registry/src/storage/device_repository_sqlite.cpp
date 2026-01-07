#include "aco/storage/device_repository.hpp"
#include <sqlite3.h>
#include <string>
#include <memory>

namespace aco::storage {

static sqlite3* g_db = nullptr;

Result<void> DeviceRepository::init(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &g_db) != SQLITE_OK) {
        return Result<void>::error("Cannot open database: " + std::string(sqlite3_errmsg(g_db)));
    }

    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS devices (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            serial_number TEXT NOT NULL UNIQUE,
            model TEXT NOT NULL,
            owner TEXT,
            public_key_pem TEXT,
            status TEXT NOT NULL DEFAULT 'PENDING',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            activated_at TIMESTAMP,
            last_seen_at TIMESTAMP,
            CHECK(status IN ('PENDING', 'ACTIVE', 'REVOKED'))
        );
    )";

    if (sqlite3_exec(g_db, schema, nullptr, nullptr, nullptr) != SQLITE_OK) {
        return Result<void>::error("Failed to create table");
    }
    return Result<void>::ok({});
}

static int callback_get(void* data, int argc, char** argv, char**) {
    auto record = static_cast<DeviceRecord*>(data);
    record->id = std::stoi(argv[0]);
    record->serial_number = argv[1];
    record->model = argv[2];
    record->owner = argv[3] ? argv[3] : "";
    record->public_key_pem = argv[4] ? argv[4] : "";
    record->status = argv[5];
    return 0;
}

Result<std::optional<DeviceRecord>> DeviceRepository::get_by_sn(std::string_view sn) {
    std::string sql = "SELECT id, serial_number, model, owner, public_key_pem, status FROM devices WHERE serial_number = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<std::optional<DeviceRecord>>::error("Prepare failed");
    }
    sqlite3_bind_text(stmt, 1, sn.data(), static_cast<int>(sn.size()), SQLITE_STATIC);

    DeviceRecord record;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        callback_get(&record, 6, 
            const_cast<char**>(reinterpret_cast<const char* const*>(&record.id)), 
            nullptr);
        // 手动赋值（简化）
        record.id = sqlite3_column_int(stmt, 0);
        record.serial_number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        record.model = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        record.owner = sqlite3_column_text(stmt, 3) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        record.public_key_pem = sqlite3_column_text(stmt, 4) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        record.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        sqlite3_finalize(stmt);
        return Result<std::optional<DeviceRecord>>::ok(record);
    }
    sqlite3_finalize(stmt);
    return Result<std::optional<DeviceRecord>>::ok(std::nullopt);
}

Result<void> DeviceRepository::pre_register(const DeviceRecord& dev) {
    std::string sql = R"(
        INSERT INTO devices (serial_number, model, owner, public_key_pem)
        VALUES (?, ?, ?, ?)
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(g_db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return Result<void>::error("Prepare insert failed");
    }
    sqlite3_bind_text(stmt, 1, dev.serial_number.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dev.model.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, dev.owner.empty() ? nullptr : dev.owner.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, dev.public_key_pem.empty() ? nullptr : dev.public_key_pem.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = "Insert failed: ";
        err += sqlite3_errmsg(g_db);
        sqlite3_finalize(stmt);
        return Result<void>::error(err);
    }
    sqlite3_finalize(stmt);
    return Result<void>::ok({});
}

} // namespace aco::storage