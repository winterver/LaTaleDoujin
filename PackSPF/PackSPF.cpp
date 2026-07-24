#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <vector>

using Tup = std::pair<std::string, LARGE_INTEGER>;
using tstring = std::string;
static std::vector<Tup> gFiles;

int Tree(const tstring& dir)
{
    HANDLE hFind;
    WIN32_FIND_DATA wfd;
    LARGE_INTEGER liFileSize;

    hFind = FindFirstFile((dir+"/*").c_str(), &wfd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Error, failed to open directory\n");
        ExitProcess(-1);
    }

    do
    {
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (tstring(".") == wfd.cFileName) continue;
            if (tstring("..") == wfd.cFileName) continue;
            Tree(dir+"/"+wfd.cFileName);
        }
        else
        {
            liFileSize.LowPart = wfd.nFileSizeLow;
            liFileSize.HighPart = wfd.nFileSizeHigh;
            gFiles.push_back(Tup(dir+"/"+wfd.cFileName, liFileSize));
        }
    }
    while (FindNextFile(hFind, &wfd) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES)
    {
        printf("Error, failed to iterate directory\n");
        ExitProcess(-1);
    }

    FindClose(hFind);
    return 0;
}

BOOL ConcatFile(HANDLE hTarget, const char* szFilename)
{
    HANDLE hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Error, skipped %s\n", szFilename);
        return FALSE;
    }

    BYTE buffer[4096];
    DWORD nBytesRead;
    DWORD nBytesWritten;

    while (ReadFile(hFile, buffer, sizeof(buffer), &nBytesRead, NULL) && nBytesRead != 0)
    {
        WriteFile(hTarget, buffer, nBytesRead, &nBytesWritten, NULL);
    }

    CloseHandle(hFile);

    return TRUE;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        printf("Usage: %s <packfile name> <directory name>\n", argv[0]);
        return -1;
    }

    HANDLE hFile = CreateFile(argv[1], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Error, failed to create %s\n", argv[1]);
        return -1;
    }

    Tree(argv[2]);

    struct entry {
        char szFilename[128] = { };
        int offset = 0;
        int size = 0;
        int index = 0;
    };
    std::vector<entry> meta;

    int offset = 0;
    for (const auto& pair : gFiles)
    {
        if (ConcatFile(hFile, pair.first.c_str()))
        {
            entry e;
            memcpy_s(e.szFilename, sizeof(e.szFilename)-1, pair.first.c_str(), pair.first.length());
            e.size = pair.second.QuadPart;
            e.offset = offset;
            offset += e.size;
            meta.push_back(e);
        }
    }

    DWORD nBytesWritten;
    entry e;

    WriteFile(hFile, meta.data(), meta.size()*sizeof(entry), &nBytesWritten, NULL);
    WriteFile(hFile, &e, sizeof(e), &nBytesWritten, NULL);

    CloseHandle(hFile);
}