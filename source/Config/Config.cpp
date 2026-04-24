#include "Config.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Config getDefaultConfig()
{
    return {
        "http://localhost:11434/api/generate",
        "qwen3.6:35b",
        "You are a professional game localizer. "
        "Please translate the following string from a .po file to {language} "
        "while preserving the format, maintaining all special characters and formatting "
        "as required for a valid .po file entry. "
        "Do not translate or modify any text inside curly braces {like_this}; keep it exactly as it appears. "
        "Do not output errors or explanations.",
        true};
}

Config loadConfig(const std::string &path)
{
    if (!std::filesystem::exists(path))
    {
        Config def = getDefaultConfig();
        saveConfig(def, path);
        return def;
    }

    std::ifstream file(path);
    json j;
    file >> j;

    return {
        j.value("apiUrl", "http://localhost:11434/api/generate"),
        j.value("model", "qwen3.6:35b"),
        j.value("prompt", "You are a professional game localizer. "
                         "Please translate the following string from a .po file to {language} "
                         "while preserving the format, maintaining all special characters and formatting "
                         "as required for a valid .po file entry. "
                         "Do not translate or modify any text inside curly braces {like_this}; keep it exactly as it appears. "
                         "Do not output errors or explanations."),
        j.value("overwriteOriginalFiles", true)};
}

void saveConfig(const Config &config, const std::string &path)
{
    json j;
    j["apiUrl"] = config.apiUrl;
    j["model"] = config.model;
    j["prompt"] = config.prompt;
    j["overwriteOriginalFiles"] = config.overwriteOriginalFiles;

    std::ofstream file(path);
    file << j.dump(4);
}
