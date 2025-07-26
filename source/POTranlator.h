//All rights reserved by Daniil Grigoriev.
#pragma once

#include <string>
#include <vector>

class POTranslator
{
public:
    POTranslator(std::string language_);
    ~POTranslator() {};

    std::string StartTranslate(const std::string& input);

private:
	std::string language;

    std::string Translate(const std::string& text);

    const std::string apiUrl = "http://localhost:11434/api/generate";

    const std::string model = "qwen3:30b";

};