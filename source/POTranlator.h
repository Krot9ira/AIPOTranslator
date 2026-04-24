// All rights reserved by Daniil Grigoriev.
#pragma once

#include <string>
#include <vector>
#include "Config/Config.h"

class POTranslator
{
public:
    POTranslator(std::string language_);
    ~POTranslator() {};

    std::string StartTranslate(const std::string &input);

private:
    std::string language;

    std::string Translate(const std::string &text);

    Config config;

    std::string apiUrl;
    std::string model;
    bool overwriteOriginalFiles;
};