//All rights reserved by Daniil Grigoriev.
#pragma once

#include <string>
#include <vector>
#include "nlohmann\json.hpp"

using namespace std::filesystem;

void SplitString(const std::string& input, std::vector<std::string>& subStrings);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

void TranslatePO(const path& sourceFile, const path& sourceRoot, bool overwriteOriginal);

void FindAllPO(const path& sourceDir);

void GetAllMsgstrs(const path& sourceFile, std::vector<std::string>& msgstrs);

const bool overwriteOriginalFiles = false;