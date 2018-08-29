#include "sysio.h"
#include <algorithm>    // for std::replace

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dirent.h>     // To search files inside a directory
    #include <cstring>
#endif
#include <sys/stat.h>


void standardizePath(std::string &s)
{
    std::replace(s.begin(), s.end(), '\\', '/');
    if (s[s.length() - 1] != '/')
        s += '/';
}

bool isValidFile(const std::string& filePath)
{
    struct stat info;

    if (stat( filePath.c_str(), &info ) != 0)
        return false;
    else
        return true;
}

bool isValidDirectory(const std::string& directoryPath)
{
    struct stat info;

    if (stat( directoryPath.c_str(), &info ) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

void getFilesInDirectorySub(std::vector<std::string> *out, std::string directory)
{
    // This function checks recursively in the given folder.
    // TODO: right now doesn't get saves and .ini.
    standardizePath(directory);

#ifdef _WIN32
    HANDLE dir;
    WIN32_FIND_DATAA findData;

    if ((dir = FindFirstFileA((directory + '*').c_str(), &findData)) == INVALID_HANDLE_VALUE)
        return;     // No files found

    do
    {
        const std::string file_name = findData.cFileName;
        const std::string full_file_name = directory + file_name;
        const bool is_directory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (is_directory)
        {
            // Recurse this directory
            if ( (strcmp(file_name.c_str(), "..") == 0) || (strcmp(file_name.c_str(), ".") == 0) )
                continue;   // Ignore this one

            getFilesInDirectorySub(out, full_file_name);
            continue;
        }
        else if (file_name[0] == '.')
            continue;
        else if (file_name.length() <= 4)
            continue;
        else if (strcmp(file_name.c_str() + file_name.length() - 4, ".scp") != 0)
            continue;   // we look only for .scp files.

        out->push_back(full_file_name);
    } while (FindNextFileA(dir, &findData));

    FindClose(dir);
#else
    DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL)
    {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = directory + "/" + file_name;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;
        const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        if (is_directory)
        {
            // Recurse this directory
            if ( (strcmp(file_name.c_str(), "..") == 0) || (strcmp(file_name.c_str(), ".") == 0) )
                continue;   // Ignore this one

            getFilesInDirectorySub(out, full_file_name);
            continue;
        }
        else if (file_name[0] == '.')
            continue;
        else if (file_name.length() <= 4)
            continue;
        else if (strcmp(file_name.c_str() + file_name.length() - 4, ".scp") != 0)
            continue;   // we look only for .scp files.

        out->push_back(full_file_name);
    }
    closedir(dir);
#endif
}
