// All rights reserved by Daniil Grigoriev.

#include "POTranlator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <curl\curl.h>
#include "ThirdParty\nlohmann\json.hpp"
#include <Shlobj.h>
#include <chrono>
#include <map>
#include <regex>
#include <algorithm>
#include <cctype>
#include <xlnt/xlnt.hpp>
#include <zip.h>
#include "Utility.h"

using json = nlohmann::json;
using namespace std::filesystem;

// For curl response
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    if (!userp)
        return 0;

    userp->append(static_cast<char *>(contents), size * nmemb);

    return size * nmemb;
}

void SplitString(const std::string &input, std::vector<std::string> &subStrings)
{
    size_t start = 0;
    size_t end = input.find("\\r\\n");

    while (end != std::string::npos)
    {
        subStrings.push_back(input.substr(start, end - start));
        start = end + 4; // Skip "\r\n"
        end = input.find("\\r\\n", start);
    }

    subStrings.push_back(input.substr(start));
}

void TranslatePO(const path &sourceFile, const path &sourceRoot, bool overwriteOriginal, const ProgressCallback &progressCallback, const bool& bCancelled)
{
#pragma region Line_Counting
    std::ifstream counter(sourceFile);
    size_t totalLines = 0;
    std::string tmp;
    while (std::getline(counter, tmp))
        totalLines++;
    counter.close();

    if (totalLines == 0)
    {
        std::cerr << "File is empty or cannot count lines." << std::endl;
        return;
    }
#pragma endregion

    path sourceDir = sourceFile.parent_path();

    std::string stem = sourceFile.stem().string();
    std::string ext = sourceFile.extension().string();

    path outputPath;
    if (overwriteOriginal)
    {
        outputPath = sourceDir / (stem + "_tmpTranslated" + ext);
    }
    else
    {
        outputPath = sourceDir / (stem + "_Translated" + ext);
    }

    std::ifstream input(sourceFile);
    std::ofstream output(outputPath);

    if (!input)
    {
        std::cerr << "Failed to open input file." << std::endl;
        return;
    }

    if (!output)
    {
        std::cerr << "Failed to open output file." << std::endl;
        return;
    }

    std::string language = sourceFile.parent_path().filename().string();
    POTranslator tranlator(language);

    std::string line, msgid, msgstr;
    bool inMsgid = false, inMsgstr = false;

    size_t processedLines = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (std::getline(input, line) && !bCancelled)
    {
        processedLines++;
        size_t percent = (processedLines * 100) / totalLines;
        if (progressCallback)
        {
            progressCallback(percent, language + "/" + sourceFile.filename().string());
        }
        auto now = std::chrono::steady_clock::now();
        double elapsedSec = std::chrono::duration<double>(now - startTime).count();
        double etaSec = (processedLines > 0 ? elapsedSec * (double(totalLines) / processedLines - 1.0) : 0.0);

        int etaMinutes = int(etaSec) / 60;
        int etaSeconds = int(etaSec) % 60;

        int barWidth = 40;
        size_t filled = (percent * barWidth) / 100;

        std::cout << "\r[";
        for (int i = 0; i < filled; i++)
            std::cout << "#";
        for (size_t i = filled; i < barWidth; i++)
            std::cout << "-";
        std::cout << "] " << percent << "% ("
                  << processedLines << "/" << totalLines << ")";

        if (etaMinutes > 0)
            std::cout << "  ETA: " << etaMinutes << "m " << etaSeconds << "s   ";
        else
            std::cout << "  ETA: " << etaSeconds << "s   ";

        std::cout.flush();

        if (line.rfind("msgid ", 0) == 0)
        {
            msgid = line.substr(6);
            msgid = msgid.substr(1, msgid.length() - 2);
            output << "msgid \"" << msgid << "\"\n";
            inMsgid = true;
            inMsgstr = false;
        }
        else if (line.rfind("msgstr ", 0) == 0 && inMsgid)
        {
            msgstr = line.substr(7);
            msgstr = msgstr.substr(1, msgstr.length() - 2);

            if (msgstr.empty())
            {
                msgstr = tranlator.StartTranslate(msgid);
            }

            output << "msgstr \"" << msgstr << "\"\n";
            inMsgstr = true;
        }
        else
        {
            output << line << "\n";
        }
    }

    std::cout << std::endl;
    input.close();
    output.close();

    if (overwriteOriginal)
    {
        if (!bCancelled)
        {
            try
            {
                std::filesystem::remove(sourceFile);
                std::filesystem::rename(outputPath, sourceFile);
                std::cout << "Original file replaced successfully.\n";
            }
            catch (std::exception& e)
            {
                std::cerr << "Failed to replace original file: " << e.what() << std::endl;
            }
        }
        else
        {
            try
            {
                std::filesystem::remove(outputPath);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Output file cant be removed or doesnt exist: " << e.what() << std::endl;
            }
            
        }
        
    }
}

void FindAllPO(const path &sourceDir, bool overwriteOriginalFiles, const ProgressCallback &progressCallback, const bool& bCancelled)
{
    if (!exists(sourceDir) || !is_directory(sourceDir))
    {
        std::cerr << "Source directory does not exist or is not a directory.\n";
        return;
    }

    for (const auto &entry : recursive_directory_iterator(sourceDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".po" && !bCancelled)
        {
            TranslatePO(entry.path(), sourceDir, overwriteOriginalFiles, progressCallback, bCancelled);
        }
        else if (bCancelled)
        {
            std::cerr << "Translation canceled\n";
            break;
        }
    }
}

void DiscoverPOFiles(const path &sourceDir, std::vector<std::string> &poFiles)
{
    poFiles.clear();

    if (!exists(sourceDir) || !is_directory(sourceDir))
    {
        return;
    }

    for (const auto &entry : recursive_directory_iterator(sourceDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".po")
        {
            poFiles.push_back(entry.path().string());
        }
    }
}

// ---- XLSX translation ----

static std::string ToLower(const std::string &s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

static bool ParseRGB(const std::string &hex, int &r, int &g, int &b)
{
    // Accept "AARRGGBB" (8) or "RRGGBB" (6)
    std::string h = hex;
    if (h.size() == 8)
        h = h.substr(2);
    if (h.size() != 6)
        return false;
    try
    {
        r = std::stoi(h.substr(0, 2), nullptr, 16);
        g = std::stoi(h.substr(2, 2), nullptr, 16);
        b = std::stoi(h.substr(4, 2), nullptr, 16);
        return true;
    }
    catch (const std::exception &)
    {
        return false;
    }
}

// Extract a single color (as "RRGGBB") from one theme color element, e.g. the
// content between <a:accent1> and </a:accent1>. Handles both srgbClr and sysClr.
static std::string ExtractColorFromSegment(const std::string &seg)
{
    auto pickAttr = [&](const std::string &key) -> std::string {
        size_t p = seg.find(key);
        if (p == std::string::npos)
            return "";
        p += key.size();
        size_t e = seg.find('"', p);
        if (e == std::string::npos)
            return "";
        return seg.substr(p, e - p);
    };

    std::string v = pickAttr("srgbClr val=\"");
    if (v.size() >= 6)
        return v.substr(v.size() - 6); // strip optional leading alpha
    // sysClr (e.g. windowText / window) carries the resolved color in lastClr.
    v = pickAttr("lastClr=\"");
    if (v.size() >= 6)
        return v.substr(v.size() - 6);
    return "";
}

// Read xl/theme/theme1.xml from the .xlsx and return the 12 theme colors in the
// SpreadsheetML theme-index order (0=lt1,1=dk1,2=lt2,3=dk2,4..9=accent1..6,
// 10=hlink,11=folHlink). Empty entries mean "could not resolve".
static std::vector<std::string> LoadThemeColors(const std::string &xlsxPath)
{
    std::vector<std::string> result(12);

    int err = 0;
    zip_t *za = zip_open(xlsxPath.c_str(), ZIP_RDONLY, &err);
    if (!za)
        return result;

    std::string xml;
    zip_file_t *zf = zip_fopen(za, "xl/theme/theme1.xml", 0);
    if (zf)
    {
        char buf[8192];
        zip_int64_t n;
        while ((n = zip_fread(zf, buf, sizeof(buf))) > 0)
            xml.append(buf, static_cast<size_t>(n));
        zip_fclose(zf);
    }
    zip_close(za);

    if (xml.empty())
        return result;

    // Limit parsing to the <a:clrScheme> ... </a:clrScheme> block.
    size_t schemeStart = xml.find("clrScheme");
    size_t schemeEnd = xml.find("/a:clrScheme");
    if (schemeEnd == std::string::npos)
        schemeEnd = xml.find("</clrScheme");
    std::string scheme = (schemeStart != std::string::npos)
                             ? xml.substr(schemeStart, (schemeEnd != std::string::npos ? schemeEnd - schemeStart : std::string::npos))
                             : xml;

    // Color elements in document order.
    const char *names[12] = {"dk1", "lt1", "dk2", "lt2",
                             "accent1", "accent2", "accent3",
                             "accent4", "accent5", "accent6",
                             "hlink", "folHlink"};
    std::vector<std::string> docOrder(12);
    for (int i = 0; i < 12; ++i)
    {
        std::string open = std::string(":") + names[i] + ">";
        size_t p = scheme.find(open);
        if (p == std::string::npos)
        {
            open = std::string("<") + names[i] + ">";
            p = scheme.find(open);
        }
        if (p == std::string::npos)
            continue;
        size_t segStart = p;
        size_t segEnd = scheme.find(names[i], segStart + open.size()); // closing tag
        std::string seg = scheme.substr(segStart, (segEnd != std::string::npos ? segEnd - segStart : 256));
        docOrder[i] = ExtractColorFromSegment(seg);
    }

    // Remap document order -> theme index order (dk1/lt1 and dk2/lt2 swapped).
    result[0] = docOrder[1];  // lt1
    result[1] = docOrder[0];  // dk1
    result[2] = docOrder[3];  // lt2
    result[3] = docOrder[2];  // dk2
    for (int i = 4; i < 12; ++i)
        result[i] = docOrder[i]; // accent1..6, hlink, folHlink

    return result;
}

// Resolve one xlnt color to RGB, using the theme palette for theme-indexed colors.
static bool ResolveColorRGB(const xlnt::color &col, const std::vector<std::string> &themeColors,
                            int &r, int &g, int &b)
{
    try
    {
        if (col.type() == xlnt::color_type::rgb)
        {
            return ParseRGB(col.rgb().hex_string(), r, g, b);
        }
        if (col.type() == xlnt::color_type::theme)
        {
            std::size_t idx = col.theme().index();
            if (idx < themeColors.size() && !themeColors[idx].empty())
                return ParseRGB(themeColors[idx], r, g, b);
        }
    }
    catch (const std::exception &)
    {
    }
    return false;
}

// Get the visible fill color of a cell as RGB. For a solid fill the foreground
// is the displayed color; we fall back to the background if needed.
// NOTE: colors coming from conditional formatting are not resolved and ignored.
static bool GetCellFillRGB(const xlnt::cell &cell, const std::vector<std::string> &themeColors,
                           int &r, int &g, int &b)
{
    try
    {
        if (!cell.has_format())
            return false;

        // xlnt's fill::type() can report "none" even for real solid fills, so we
        // read the pattern_fill directly.
        xlnt::pattern_fill pf = cell.fill().pattern_fill();
        if (pf.type() != xlnt::pattern_fill_type::solid)
            return false;

        if (pf.foreground().is_set() && ResolveColorRGB(pf.foreground().get(), themeColors, r, g, b))
            return true;
        if (pf.background().is_set() && ResolveColorRGB(pf.background().get(), themeColors, r, g, b))
            return true;
    }
    catch (const std::exception &)
    {
    }
    return false;
}

static bool IsGreenFill(const xlnt::cell &cell, const std::vector<std::string> &themeColors)
{
    int r, g, b;
    if (GetCellFillRGB(cell, themeColors, r, g, b))
        return (g > r && g > b && g >= 0x80);
    return false;
}

static bool IsRedFill(const xlnt::cell &cell, const std::vector<std::string> &themeColors)
{
    int r, g, b;
    if (GetCellFillRGB(cell, themeColors, r, g, b))
        return (r > g && r > b && r >= 0x80);
    return false;
}

void TranslateXLSX(const path &sourceFile, bool translateInPlace, const ProgressCallback &progressCallback, const bool &bCancelled)
{
    xlnt::workbook wb;
    try
    {
        wb.load(sourceFile.string());
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Failed to load xlsx '" << sourceFile.string() << "': " << ex.what() << std::endl;
        return;
    }

    // Theme palette is needed to resolve theme-indexed cell fill colors.
    std::vector<std::string> themeColors = LoadThemeColors(sourceFile.string());

    // Snapshot original sheet titles (we may add copies during processing).
    std::vector<std::string> originalTitles = wb.sheet_titles();

    // Case-insensitive lookup of title -> actual title for base-sheet resolution.
    std::map<std::string, std::string> titleByLower;
    for (const auto &t : originalTitles)
        titleByLower[ToLower(t)] = t;

    const std::regex suffixRe(R"(^(.+)-([A-Za-z]{2,})$)");

    // Estimate total work for progress (cells in target sheets).
    size_t totalCells = 0;
    for (const auto &title : originalTitles)
    {
        std::smatch m;
        if (!std::regex_match(title, m, suffixRe))
            continue;
        xlnt::worksheet ws = wb.sheet_by_title(title);
        totalCells += static_cast<size_t>(ws.highest_row()) * static_cast<size_t>(ws.highest_column().index);
    }
    if (totalCells == 0)
        totalCells = 1;

    std::map<std::string, POTranslator> translators;
    size_t processedCells = 0;
    bool anyChange = false;

    for (const auto &title : originalTitles)
    {
        if (bCancelled)
            break;

        std::smatch m;
        if (!std::regex_match(title, m, suffixRe))
            continue; // No "-CODE" suffix -> ignore this sheet.

        std::string baseName = m[1].str();
        std::string code = m[2].str();

        // Resolve optional base sheet (same name without suffix, case-insensitive).
        bool hasBase = false;
        xlnt::worksheet baseWs;
        auto baseIt = titleByLower.find(ToLower(baseName));
        if (baseIt != titleByLower.end())
        {
            baseWs = wb.sheet_by_title(baseIt->second);
            hasBase = true;
        }

        // Pick the sheet to write into.
        xlnt::worksheet targetWs = wb.sheet_by_title(title);
        if (!translateInPlace)
        {
            xlnt::worksheet copy = wb.copy_sheet(targetWs);
            copy.title(title + " (Translated)");
            targetWs = copy;
        }

        // Translator for this language code (cached per workbook).
        auto trIt = translators.find(code);
        if (trIt == translators.end())
            trIt = translators.emplace(code, POTranslator(code)).first;
        POTranslator &translator = trIt->second;

        auto highestRow = targetWs.highest_row();
        auto highestCol = targetWs.highest_column();
		bool sheetChange = false;
        for (xlnt::row_t row = 1; row <= highestRow && !bCancelled; ++row)
        {
            for (xlnt::column_t::index_t col = 1; col <= highestCol.index && !bCancelled; ++col)
            {
                processedCells++;
                size_t percent = (processedCells * 100) / totalCells;
                if (percent > 100)
                    percent = 100;
                if (progressCallback)
                    progressCallback(percent, sourceFile.filename().string() + " : " + title);

                xlnt::cell cell = targetWs.cell(xlnt::column_t(col), row);

                auto dt = cell.data_type();
                if (dt == xlnt::cell_type::empty)
                    continue;

                std::string text = cell.to_string();
                if (text.empty())
                    continue;

                bool isNumber = (dt == xlnt::cell_type::number || dt == xlnt::cell_type::boolean);

                // Rule 1 already handled (empty skipped).
                // Rule 2: green fill -> never translate.
                if (IsGreenFill(cell, themeColors))
                    continue;

                // Rule 3: red fill + text -> always translate.
                bool forceTranslate = (IsRedFill(cell, themeColors) && !isNumber);

                if (!forceTranslate)
                {
                    // Only text cells are eligible (numbers skipped).
                    if (isNumber)
                        continue;

                    // Rule 4: translate only if still matching the base sheet (untranslated).
                    if (hasBase)
                    {
                        xlnt::cell baseCell = baseWs.cell(xlnt::column_t(col), row);
                        if (baseCell.to_string() != text)
                            continue; // already differs -> assume already translated.
                    }
                    // If no base sheet, fall through and translate.
                }

                std::string translated = translator.StartTranslate(text);
                if (!translated.empty() && translated != text)
                {
                    cell.value(translated);
                    anyChange = true;
					sheetChange = true;
                    std::cout << cell.reference().to_string() << " [" << title << "]: "
                              << text << " -> " << translated << std::endl;
                }
            }
        }
        if (sheetChange)
        {
            try
            {
                wb.save(sourceFile.string());
                std::cout << "Saved translated xlsx: " << sourceFile.string() << std::endl;
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Failed to save xlsx '" << sourceFile.string() << "': " << ex.what() << std::endl;
            }
        }
    }

    if (bCancelled)
    {
        std::cerr << "XLSX translation cancelled, not saving '" << sourceFile.string() << "'" << std::endl;
        return;
    }

    if (anyChange)
    {
        try
        {
            wb.save(sourceFile.string());
            std::cout << "Saved translated xlsx: " << sourceFile.string() << std::endl;
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to save xlsx '" << sourceFile.string() << "': " << ex.what() << std::endl;
        }
    }
}

void GetAllMsgstrs(const path &sourceFile, std::vector<std::string> &msgstrs)
{
    path sourceDir = sourceFile.parent_path();
    std::ifstream input(sourceFile);
    if (!input)
    {
        std::cerr << "Failed to open input file." << std::endl;

        if (input.bad())
        {
            std::cerr << "Fatal error: badbit is set." << std::endl;
        }

        if (input.fail())
        {
            char buff[256];
            std::cerr << "Error details: " << strerror_s(buff, 100, 0) << buff
                      << std::endl;
        }
        return;
    }

    std::string line;
    bool inMsgid = false, inMsgstr = false;

    while (std::getline(input, line))
    {
        std::string msgid, msgstr;
        if (line.rfind("msgid ", 0) == 0)
        {
            msgid = line.substr(6);
            msgid = msgid.substr(1, msgid.length() - 2); // Delete quotes
            inMsgid = true;
            inMsgstr = false;
        }
        else if (line.rfind("msgstr ", 0) == 0 && inMsgid)
        {
            msgstr = line.substr(7);
            msgstr = msgstr.substr(1, msgstr.length() - 2); // Delete quotes

            if (msgstr.empty())
            {
                msgstrs.push_back(msgstr);
            }

            inMsgstr = true;
        }
    }

    input.close();
}
