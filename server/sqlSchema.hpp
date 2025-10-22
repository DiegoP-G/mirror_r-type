#pragma once

constexpr char sqlSchema[] = R"(
    CREATE TABLE IF NOT EXISTS Players (
        ID INTEGER PRIMARY KEY AUTOINCREMENT,
        Name TEXT NOT NULL,
        Password TEXT NOT NULL,
        Score INTEGER DEFAULT 0
    );

    CREATE TABLE IF NOT EXISTS BannedIp (
        ID INTEGER PRIMARY KEY AUTOINCREMENT,
        Ip VARCHAR(16) NOT NULL
    );
)";