//All rights reserved by Daniil Grigoriev.

#include "POTranlator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl\curl.h>
#include "Utility.h"
#include "nlohmann\json.hpp"

using json = nlohmann::json;

POTranslator::POTranslator(std::string language_)
{
    language = language_;
    std::cout << "Start translating to " << language << " ..." << std::endl;
}


// Translate call
std::string POTranslator::Translate(const std::string& text) {
    if (text.empty()) {
        // nothing to translate
        return "";
    }
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::ostringstream ossPrompt;
    ossPrompt << "Please translate the following string from.po file to " << language << " while preserving the format, maintaining all special characters and formatting as required for a valid.po file entry.";
    std::string prompt = ossPrompt.str();

    prompt.append("'");
    prompt.append(text);
    prompt.append("'");
    prompt.append(". Respond using JSON");


    
	// We dont want stream because we just generating transltation and not chating with the model
    bool stream = false;
	//For now disabling think, as it will take more time to process the request and actiualy have no effect on the translation
    bool think = false;

    std::string format = "json";

    std::ostringstream ossBody;
	// Constructing the JSON body for the request for more information go to Ollama API documentation
    ossBody << "{\"model\": \"" << model << "\", \"prompt\": \"" << prompt << "\", \"think\": "<< (think ? "true" : "false") << ", \"stream\": " << (stream ? "true" : "false") << ", \"format\": \"" << format << "\"}";
    std::string body = ossBody.str();

    // Requesting translate and check response for translation
    curl = curl_easy_init();
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);


        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }


        json response_json = json::parse(readBuffer);


        curl_easy_cleanup(curl);


        if (response_json.contains("response")) {
            std::string pojson = response_json["response"];
            json finalmgstr = json::parse(pojson);
            if (finalmgstr.contains("msgstr")) {
                std::string translatedText = finalmgstr["msgstr"];
                std::cout << translatedText << "  ==  " << text << std::endl;
                return translatedText;
            }
            else if (finalmgstr.contains("translated"))
            {
                std::string translatedText = finalmgstr["translated"];
                std::cout << translatedText << "  ==  " << text << std::endl;
                return translatedText;
            }
            else if (finalmgstr.contains("translation"))
            {
                std::string translatedText = finalmgstr["translation"];
                std::cout << translatedText << "  ==  " << text << std::endl;
                return translatedText;
            }
            else if (finalmgstr.contains("translated_string"))
            {
                std::string translatedText = finalmgstr["translated_string"];
                std::cout << translatedText << "  ==  " << text << std::endl;
                return translatedText;
            }
            else
            {
                return "";
            }
        }
        else {
            std::cerr << "No 'response' field" << std::endl;
            return "";
        }
    }

    return "";
}

std::string POTranslator::StartTranslate(const std::string& input) {
    if (input.empty())
    {
        return input;
    }
    std::cout << "translating new line..." << std::endl;
    std::vector<std::string> subStrings;
	SplitString(input, subStrings);
    std::string result;
    for (size_t i = 0; i < subStrings.size(); i++)
    {
        result.append(Translate(subStrings[i]));
        if (i != subStrings.size() - 1)
        {
            result.append("\\r\\n");
        }
    }
    if (result.empty())
    {
        result = input;
    }
    std::cout << "End translating line..." << std::endl;
    return result;
}


