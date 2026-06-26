#pragma once
#include <string>

struct Config
{
    std::string apiUrl;
    std::string model;
    std::string prompt;
    bool overwriteOriginalFiles = true;
    bool translateInPlace = true;
};

Config getDefaultConfig();
Config loadConfig(const std::string &path);
void saveConfig(const Config &config, const std::string &path);