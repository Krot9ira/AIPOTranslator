//All rights reserved by Daniil Grigoriev.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include "POTranlator.h"
#include <curl\curl.h>
#include <Shlobj.h>
#include "Utility.h"




int main() {

    SetConsoleOutputCP(CP_UTF8);
    BROWSEINFOW bi = { 0 }; 
    wchar_t path[MAX_PATH];

    bi.lpszTitle = L"Select a folder"; 
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.pszDisplayName = path; 

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl != NULL)
    {
        if (SHGetPathFromIDListW(pidl, path)) 
        {
            FindAllPO(path);
        }

        CoTaskMemFree(pidl);
    }

    curl_global_cleanup();

    return 0;
}
