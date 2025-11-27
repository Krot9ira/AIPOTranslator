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

    const std::string model = "qwen3:30b-instruct"; //for european cultures same results with higher speed give gpt-oss:20b //qwen3:30b-a3b-instruct-2507-q8_0 //qwen3:30b-instruct

};