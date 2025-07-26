
#include <string>
#include <iostream>
#include "POTranlator.h"
#include "Utility.h"
#include <fstream>

bool CompareTranslations(const std::string& filePath1, const std::string& filePath2) {
	std::vector<std::string> lines1, lines2;
	GetAllMsgstrs(filePath1, lines1);
	GetAllMsgstrs(filePath2, lines2);
	return lines1 == lines2;
}

int main() {
	
	std::vector<std::string> output;
	SplitString("Some text for test1 \\r\\nSome text for test2", output);
	std::vector<std::string> expectedOutput = { "Some text for test1 ", "Some text for test2" };
	if (output == expectedOutput)
	{
		std::cout << "SplitString function works correctly!" << std::endl;
	}
	else
	{
		std::cout << "SplitString function failed!" << std::endl;
		std::cin.get();
		return 0;
	}
	FindAllPO("poForTests");

	if (CompareTranslations("poForTests/ru/RightTranslation.po", "poForTests/ru/ForTestTranslation_Translated.po") && CompareTranslations("poForTests/ru/RightTranslation.po", "poForTests/ru/RightTranslation_Translated.po")) {
		std::cout << "Translated correctly" << std::endl;
	}
	else {
		std::cout << "Translatied wrong" << std::endl;
	}

	const std::vector<std::string> filesToDelete = { "poForTests/ru/ForTestTranslation_Translated.po", "poForTests/ru/RightTranslation_Translated.po"};
	for (std::string filePath : filesToDelete)
	{
		if (std::filesystem::remove(filePath)) {
			std::cout << "Temp file cleaned " << filePath << std::endl;
		}
		else {
			std::cout << "No temp files " << filePath << std::endl;
		}
	}
	

	std::cin.get();
    return 0;
}