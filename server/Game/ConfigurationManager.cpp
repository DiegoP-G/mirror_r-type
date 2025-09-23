#include "ConfigurationManager.hpp"
#include "../Include/Errors/ConfigurationErrors.hpp"
#include <libconfig.h++>

void Orchestrator::ConfigurationManager::loadConfiguration(const std::string configPath)
{
    try
    {
        globalConfig.readFile(configPath.c_str());
    }
    catch (const libconfig::FileIOException &)
    {
        throw ConfigurationFileException(configPath);
    }
    catch (const libconfig::ParseException &pex)
    {
        throw ConfigurationParseException(std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " +
                                          pex.getError());
    }
}

std::string Orchestrator::ConfigurationManager::getStringValue(const std::string key)
{
    try
    {
        std::string value;
        if (!globalConfig.lookupValue(key, value))
        {
            throw ConfigurationKeyNotFoundException(key);
        }
        return value;
    }
    catch (const libconfig::SettingTypeException &)
    {
        throw ConfigurationTypeException(key);
    }
}

int Orchestrator::ConfigurationManager::getIntValue(const std::string key)
{
    try
    {
        int value;
        if (!globalConfig.lookupValue(key, value))
        {
            throw ConfigurationKeyNotFoundException(key);
        }
        return value;
    }
    catch (const libconfig::SettingTypeException &)
    {
        throw ConfigurationTypeException(key);
    }
}

void Orchestrator::ConfigurationManager::modifyEntry(const std::string key, const std::string newValue)
{
    try
    {
        libconfig::Setting &root = globalConfig.getRoot();
        size_t pos = 0;
        libconfig::Setting *setting = &root;
        std::string token;
        std::string path = key;

        while ((pos = path.find('.')) != std::string::npos)
        {
            token = path.substr(0, pos);
            setting = &(*setting)[token];
            path.erase(0, pos + 1);
        }

        (*setting)[path] = newValue;
    }
    catch (const libconfig::SettingNotFoundException &)
    {
        throw ConfigurationKeyNotFoundException(key);
    }
    catch (const libconfig::SettingTypeException &)
    {
        throw ConfigurationTypeException(key);
    }
}
