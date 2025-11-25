//All rights reserved by Daniil Grigoriev.

#include "POTranlator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl\curl.h>
#include "nlohmann\json.hpp"
#include <Shlobj.h>
#include <chrono>
#include "Utility.h"

using json = nlohmann::json;
using namespace std::filesystem;

//For curl response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    if (!userp) return 0;

    userp->append(static_cast<char*>(contents), size * nmemb);

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
#pragma region Line_Counting

    std::ifstream counter(sourceFile);
    size_t totalLines = 0;
    std::string tmp;
    while (std::getline(counter, tmp)) totalLines++;
    counter.close();

    if (totalLines == 0) {
        std::cerr << "File is empty or cannot count lines." << std::endl;
        return;
    }
#pragma endregion
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

    size_t processedLines = 0;
    auto startTime = std::chrono::steady_clock::now();
    while (std::getline(input, line)) {
#pragma region Progress_Bar
        processedLines++;
        int percent = (processedLines * 100) / totalLines;
#pragma region ETA_Calculation
        auto now = std::chrono::steady_clock::now();
        double elapsedSec = std::chrono::duration<double>(now - startTime).count();

        double etaSec = 0.0;
        if (processedLines > 0) {
            etaSec = elapsedSec * (double(totalLines) / processedLines - 1.0);
        }

        int etaMinutes = int(etaSec) / 60;
        int etaSeconds = int(etaSec) % 60;
#pragma endregion
        int barWidth = 40;
        int filled = (percent * barWidth) / 100;

        std::cout << "\r[";
        for (int i = 0; i < filled; i++) std::cout << "#";
        for (int i = filled; i < barWidth; i++) std::cout << "-";
        std::cout << "] " << percent << "% ("
            << processedLines << "/" << totalLines << ")";

        if (etaMinutes > 0)
            std::cout << "  ETA: " << etaMinutes << "m " << etaSeconds << "s   ";
        else
            std::cout << "  ETA: " << etaSeconds << "s   ";

        std::cout.flush();
#pragma endregion progress bar
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
    std::cout << std::endl;
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
