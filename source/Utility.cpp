//All rights reserved by Daniil Grigoriev.

#include "POTranlator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl\curl.h>
#include "nlohmann\json.hpp"
#include <Shlobj.h>
#include "Utility.h"

using json = nlohmann::json;
using namespace std::filesystem;

//For curl response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    if (userp == nullptr) {
        return 0;
    }
    userp->reserve(size * nmemb);
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void SplitString(const std::string& input, std::vector<std::string>& subStrings)
{
    size_t start = 0;
    size_t end = input.find("\\r\\n");

    while (end != std::string::npos) {
        subStrings.push_back(input.substr(start, end - start));
        start = end + 4; // Skip "\r\n"
        end = input.find("\\r\\n", start);
    }

    subStrings.push_back(input.substr(start));
}

void TranslatePO(const path& sourceFile, const path& sourceRoot) {



    path sourceDir = sourceFile.parent_path();


    std::string stem = sourceFile.stem().string();
    std::string ext = sourceFile.extension().string();
    path translatedFileName = stem + "_Translated" + ext;


    path outputPath = sourceDir / translatedFileName;

    std::ifstream input(sourceFile);
    std::ofstream output(outputPath);




    if (!input) {
        std::cerr << "Failed to open input file." << std::endl;

        if (input.bad()) {
            std::cerr << "Fatal error: badbit is set." << std::endl;
        }

        if (input.fail()) {
            char buff[256];
            std::cerr << "Error details: " << strerror_s(buff, 100, 0) << buff
                << std::endl;
        }
        return;
    }

    if (!output) {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }
    std::string language = sourceFile.parent_path().filename().string();
    POTranslator tranlator(language);

    std::string line, msgid, msgstr;
	std::vector<std::string> msgstrs;
    bool inMsgid = false, inMsgstr = false;

    while (std::getline(input, line)) {
        if (line.rfind("msgid ", 0) == 0) {
            msgid = line.substr(6);
            msgid = msgid.substr(1, msgid.length() - 2);  // Delete quotes
            output << "msgid \"" << msgid << "\"" << std::endl;
            inMsgid = true;
            inMsgstr = false;
        }
        else if (line.rfind("msgstr ", 0) == 0 && inMsgid) {
            msgstr = line.substr(7);
            msgstr = msgstr.substr(1, msgstr.length() - 2);  // Delete quotes

            if (msgstr.empty()) {
                msgstr = tranlator.StartTranslate(msgid);
            }

            output << "msgstr \"" << msgstr << "\"" << std::endl;
            inMsgstr = true;
        }
        else {
            output << line << std::endl;
        }
    }

    input.close();
    output.close();
}

void FindAllPO(const path& sourceDir) {
    if (!exists(sourceDir) || !is_directory(sourceDir)) {
        std::cerr << "Source directory does not exist or is not a directory.\n";
        return;
    }

    for (const auto& entry : recursive_directory_iterator(sourceDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".po") {
            TranslatePO(entry.path(), sourceDir);
        }
    }
}

void GetAllMsgstrs(const path& sourceFile, std::vector<std::string>& msgstrs)
{
    path sourceDir = sourceFile.parent_path();
    std::ifstream input(sourceFile);
    if (!input) {
        std::cerr << "Failed to open input file." << std::endl;

        if (input.bad()) {
            std::cerr << "Fatal error: badbit is set." << std::endl;
        }

        if (input.fail()) {
            char buff[256];
            std::cerr << "Error details: " << strerror_s(buff, 100, 0) << buff
                << std::endl;
        }
        return;
    }

    std::string line;
    bool inMsgid = false, inMsgstr = false;

    while (std::getline(input, line)) {
        std::string msgid, msgstr;
        if (line.rfind("msgid ", 0) == 0) {
            msgid = line.substr(6);
            msgid = msgid.substr(1, msgid.length() - 2);  // Delete quotes
            inMsgid = true;
            inMsgstr = false;
        }
        else if (line.rfind("msgstr ", 0) == 0 && inMsgid) {
            msgstr = line.substr(7);
            msgstr = msgstr.substr(1, msgstr.length() - 2);  // Delete quotes

            if (msgstr.empty()) {
                msgstrs.push_back(msgstr);
            }

            inMsgstr = true;
        }
    }

    input.close();
}
