//All rights reserved by Daniil Grigoriev.

#include "POTranlator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl\curl.h>
#include "Utility.h"
#include "nlohmann\json.hpp"

using json = nlohmann::json;

#define PRINT_EACH_LINE_TRANSLATION 0

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
    ossPrompt << "You are a professional game localizer.Please translate the following string from.po file to " << language << " while preserving the format, maintaining all special characters and formatting as required for a valid.po file entry. Do not output errors or explanations.";
    std::string prompt = ossPrompt.str();

    prompt.append("'");
    prompt.append(text);
    prompt.append("'");
    prompt.append(".Respond using ONLY JSON with the key 'msgstr' or 'translation' containing the translated text. ");


    
	// We dont want stream because we just generating transltation and not chating with the model
    bool stream = false;
	//For now disabling think, as it will take more time to process the request and actiualy have no effect on the translation
    bool think = false;

    std::string format = "json";

    std::ostringstream ossBody;
	// Constructing the JSON body for the request for more information go to Ollama API documentation
    ossBody << "{\"model\": \"" << model << "\", \"prompt\": \"" << prompt << "\", \"think\": "<< (think ? "true" : "false") << ", \"stream\": " << (stream ? "true" : "false") << ", \"format\": \"" << format << "\", \"keep_alive\": \"5m\"}";
    std::string body = ossBody.str();

    // Requesting translate and check response for translation
    curl = curl_easy_init();
    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 180L);

        res = curl_easy_perform(curl);


        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
        json response_json;
        try
        {
            response_json = json::parse(readBuffer);
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Error while parsing: " << ex.what() << std::endl;
            curl_easy_cleanup(curl);
            throw ex;
        }
        


        curl_easy_cleanup(curl);


        if (response_json.contains("response")) {
            std::string pojson = response_json["response"];
            try
            {
                json finalmgstr = json::parse(pojson);
                static const std::vector<std::string> translationKeys = {
                    "msgstr", "MSGSTR",
                    "translated", "TRANSLATED",
                    "translation", "TRANSLATION",
                    "translated_string", "TRANSLATED_STRING",
                    "text", "TEXT",
                    "result", "RESULT",
                    "output", "OUTPUT",
                    "translated_text", "TRANSLATED_TEXT",
                    "translatedContent", "TRANSLATEDCONTENT",
                    "translatedText", "TRANSLATEDTEXT",
                    "translation_text", "TRANSLATION_TEXT",
                    "translationResult", "TRANSLATIONRESULT",
                    "translation_result", "TRANSLATION_RESULT",
                    "final_translation", "FINAL_TRANSLATION",
                    "finalTranslation", "FINALTRANSLATION",
                    "response", "RESPONSE",
                    "translationOutput", "TRANSLATIONOUTPUT",
                    "translation_output", "TRANSLATION_OUTPUT",
                    "value", "VALUE",
                    "data", "DATA",
                    "content", "CONTENT",
                    "string"
                };

                for (const auto& key : translationKeys) {
                    if (finalmgstr.contains(key)) {
                        std::string translatedText = finalmgstr[key];
#if PRINT_EACH_LINE_TRANSLATION
                        std::cout << translatedText << "  ==  " << text << std::endl;
#endif
                        return translatedText;
                    }
                }

                std::cerr << "No translated field. finalmgstr: " << finalmgstr << std::endl;
                throw std::runtime_error("No translated field");
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Error while parsing: " << ex.what() << std::endl;
                throw ex;
            }

        }
        else {
            std::cerr << "No 'response' field" << std::endl;
            throw std::runtime_error("No 'response' field");
        }
    }

    return "";
}

std::string POTranslator::StartTranslate(const std::string& input) {
    if (input.empty())
    {
        return input;
    }
    try
    {
#if PRINT_EACH_LINE_TRANSLATION
        std::cout << "translating new line..." << std::endl;
#endif
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
#if PRINT_EACH_LINE_TRANSLATION
        std::cout << "End translating line..." << std::endl;
#endif
        return result;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception while translating field: " << std::endl << input << std::endl << "will be empty" << std::endl << "Error:" << ex.what() << std::endl;
        return "";
    }
}


