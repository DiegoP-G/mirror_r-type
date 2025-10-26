#pragma once
#include <iostream>
#include <../sqlite/sqlite3.h>
#include <vector>

class sqlAPI
{
  private:
    std::string _dbName;
    sqlite3 *_db;

  private:
    void createOpenDatabase();
    void closeDb();
    bool tableExists(std::string name);

  public:
    sqlAPI(std::string dbName);
    ~sqlAPI()
    {
        closeDb();
    };

    std::vector<std::pair<int, std::string>> getAllScores();
    void addPlayerEntry(const std::string &name, const std::string &password);
    bool updatePlayerScore(const std::string &name, int newScore);

    bool addBannedIp(const std::string &ip);
    std::vector<std::string> getAllBannedIps();
    bool deleteBannedIpById(int id);
    bool validateCredentials(const std::string &name, const std::string &password);
};