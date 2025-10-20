#include "sqlAPI.hpp"
#include "sqlSchema.hpp"
#include <iostream>

sqlAPI::sqlAPI(std::string name) : _dbName(name)
{
    createOpenDatabase();
}

bool sqlAPI::tableExists(std::string name)
{
    std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name='" + name + "';";
    sqlite3_stmt *stmt;
    bool exists = false;

    // Prepare the query
    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        // Check if the query returns any rows
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            exists = true;
        }
    }
    else
    {
        std::cerr << "Failed to check table existence: " << sqlite3_errmsg(_db) << std::endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    return exists;
}

void sqlAPI::createOpenDatabase()
{
    char *errMsg = nullptr;

    // Open or create the database
    int rc = sqlite3_open(_dbName.c_str(), &_db);
    if (rc)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(_db) << std::endl;
        return;
    }
    else
    {
        std::cout << "Opened database successfully!" << std::endl;
    }

    if (!tableExists("Players") || !tableExists("BannedIp"))
    {
        // Execute the schema creation
        rc = sqlite3_exec(_db, sqlSchema, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        else
        {
            std::cout << "Tables created successfully!" << std::endl;
        }
    }
    else
    {
        std::cout << "Tables already exist. Skipping creation." << std::endl;
    }
}

void sqlAPI::closeDb()
{
    if (_db)
    {
        sqlite3_close(_db);
    }
}

bool sqlAPI::addPlayerEntry(const std::string &name, const std::string &password)
{
    std::string query = "INSERT INTO Players (Name, Password, Score) VALUES (?, ?, 0);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool sqlAPI::updatePlayerScore(const std::string &name, int newScore)
{
    std::string query = "UPDATE Players SET Score = ? WHERE Name = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, newScore);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<std::pair<int, std::string>> sqlAPI::getAllScores()
{
    std::vector<std::pair<int, std::string>> scores;
    std::string query = "SELECT ID, Name FROM Players;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return scores;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        int id = sqlite3_column_int(stmt, 0);
        std::string name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        scores.emplace_back(id, name);
    }

    sqlite3_finalize(stmt);
    return scores;
}

bool sqlAPI::addBannedIp(const std::string &ip)
{
    std::string query = "INSERT INTO BannedIp (Ip) VALUES (?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, ip.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

std::vector<std::string> sqlAPI::getAllBannedIps()
{
    std::vector<std::string> ips;
    std::string query = "SELECT Ip FROM BannedIp;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return ips;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string ip = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        ips.push_back(ip);
    }

    sqlite3_finalize(stmt);
    return ips;
}

bool sqlAPI::deleteBannedIpById(int id)
{
    std::string query = "DELETE FROM BannedIp WHERE ID = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(_db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}