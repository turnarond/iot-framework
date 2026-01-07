/*
 *
 */

#pragma once


#include "dto/RegistrationDto.hpp"
#include "oatpp-sqlite/orm.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient) //<- Begin Codegen

/**
 * DeviceRegistrationDb client definitions.
 */
class DeviceRegistrationDb : public oatpp::orm::DbClient {
public:

  DeviceRegistrationDb(const std::shared_ptr<oatpp::orm::Executor>& executor)
    : oatpp::orm::DbClient(executor)
  {

    const std::string DATABASE_MIGRATIONS = "db/migrations/sqlite";
    oatpp::orm::SchemaMigration migration(executor);
    migration.addFile(1 /* start from version 1 */, DATABASE_MIGRATIONS + "/001_init.sql");
    // TODO - Add more migrations here.
    migration.migrate(); // <-- run migrations. This guy will throw on error.

    auto version = executor->getSchemaVersion();
    OATPP_LOGD("DeviceRegistrationDb", "Migration - OK. Version=%lld.", version);

  }

//   QUERY(createUser,
//         "INSERT INTO AppUser"
//         "(username, email, password, role) VALUES "
//         "(:user.username, :user.email, :user.password, :user.role);",
//         PARAM(oatpp::Object<UserDto>, user))

//   QUERY(updateUser,
//         "UPDATE AppUser "
//         "SET "
//         " username=:user.username, "
//         " email=:user.email, "
//         " password=:user.password, "
//         " role=:user.role "
//         "WHERE "
//         " id=:user.id;",
//         PARAM(oatpp::Object<UserDto>, user))

//   QUERY(getUserById,
//         "SELECT * FROM AppUser WHERE id=:id;",
//         PARAM(oatpp::Int32, id))

//   QUERY(getAllUsers,
//         "SELECT * FROM AppUser LIMIT :limit OFFSET :offset;",
//         PARAM(oatpp::UInt32, offset),
//         PARAM(oatpp::UInt32, limit))

//   QUERY(deleteUserById,
//         "DELETE FROM AppUser WHERE id=:id;",
//         PARAM(oatpp::Int32, id))

};

#include OATPP_CODEGEN_END(DbClient) //<- End Codegen
