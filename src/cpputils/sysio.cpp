#include "strings.h"
#include "sysio.h"
#include <sys/stat.h>
#include <algorithm>    // for std::replace

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dirent.h>     // To search files inside a directory
    #include <cstring>
#endif

#ifdef QT_CORE_LIB
    #include <QString>
#endif


inline static constexpr char kStdDirDelim = '/';


#ifdef QT_CORE_LIB
QString standardizePath(QString s)
{
    if (s.isEmpty())
        return s;

    std::replace(s.begin(), s.end(), '\\', kStdDirDelim);
    if (isValidDirectory(s.toStdString()))
    {
        if (s[s.length() - 1] != kStdDirDelim)
            s += kStdDirDelim;
    }

    return s;
}
#endif

std::string standardizePath(std::string s)
{
    if (s.empty())
        return s;

    std::replace(s.begin(), s.end(), '\\', kStdDirDelim);
    if (isValidDirectory(s))
    {
        if (s[s.length() - 1] != kStdDirDelim)
            s += kStdDirDelim;
    }

    return s;
}

bool isValidFile(const std::string& filePath)
{
    //return std::filesystem::is_regular_file(filePath);  // Can't use this, since it was added to MacOS only since version 10.15...
    struct stat sb;
    return (stat(filePath.c_str(), &sb) == 0);
}

bool isValidDirectory(const std::string& directoryPath)
{
    //return std::filesystem::is_directory(directoryPath); // Can't use this, since it was added to MacOS only since version 10.15...
    struct stat sb;
    return (stat(directoryPath.c_str(), &sb) == 0) && ((sb.st_mode & S_IFDIR) == S_IFDIR);
}


size_t countDirLevels(std::string const& str)
{
    return std::count(str.cbegin(), str.cend(), kStdDirDelim);
}

bool comparatorDirLevels(std::string const& lhs, std::string const& rhs)
{
    return (countDirLevels(lhs) < countDirLevels(rhs));
}

std::string getDirectoryFromString(std::string const& str)
{
    if (str.empty())
        return str;
    const std::string ret(standardizePath(str));
    return ret.substr(0, ret.find_last_of(kStdDirDelim));
}


void getFilesInDirectorySub(std::vector<std::string> *out, std::string path, int maxFolderLevel)
{
    // This function adds to the list, recursively, files inside folders.
    // TODO: right now doesn't get saves and .ini.
    path = standardizePath(path);

#ifdef _WIN32
    HANDLE dir;
    WIN32_FIND_DATAW findData;

    auto pathForFindFile = [] (std::string const& str) -> std::string {
        // Remove trailing \, otherwise it fails to detect if it's a directory...
        if (!str.empty() && (str.back() == '\\'))
            return str.substr(0, str.length() - 1);
        return str;
    };

    std::replace(path.begin(), path.end(), kStdDirDelim, '\\');
    Q_ASSERT(!path.empty());
    //Q_ASSERT((path.back() == '\\') || (path.back() == '*')); // Throws if path is not a folder
    if (path.back() == '\\')
        path.pop_back();

    std::wstring buf(stringToWideString("\\\\?\\" + pathForFindFile(path)));
    if ((dir = FindFirstFileW(buf.c_str(), &findData)) == INVALID_HANDLE_VALUE)
        return;     // No files found

    // Prepare path to append filename
    if (path.back() == '*')
        path.pop_back();    // Remove '*'

    const bool first_is_directory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
    std::string fileName;
    std::string bufNewPath;
    bool success = true;
    while (success && (dir != INVALID_HANDLE_VALUE))
    {
        fileName = wideStringToString(findData.cFileName);
        const bool cur_is_directory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;

        if (!fileName.compare("."))
            goto loop_continue;
        if (!fileName.compare(".."))
            goto loop_continue;

        if (cur_is_directory)
        {
            // Recurse this directory
            bufNewPath = path;
            if (path.back() == '\\')
                bufNewPath += fileName + '\\';
            else
                bufNewPath += '\\';

            if (maxFolderLevel != 0)
            {
                getFilesInDirectorySub(out, bufNewPath + "*", maxFolderLevel - 1);
                out->emplace_back(standardizePath(bufNewPath));
            }
            goto loop_continue;
        }

        if (fileName[0] == '.')
            goto loop_continue;
        if (fileName.length() <= 4)
            goto loop_continue;
        if (strcmp(fileName.c_str() + fileName.length() - 4, ".scp") != 0)
            goto loop_continue;   // we look only for .scp files.

        bufNewPath = path;
        if (first_is_directory && !cur_is_directory)
            bufNewPath += fileName;
        out->emplace_back(standardizePath(bufNewPath));

loop_continue:
        success = FindNextFileW(dir, &findData);
    }

    FindClose(dir);
#else
    DIR *dir = opendir(path.c_str());
    while (class dirent *ent = readdir(dir))
    {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = path + kStdDirDelim + file_name;

        class stat st;
        if (stat(full_file_name.c_str(), &st) == -1)
            continue;
        const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        if (is_directory)
        {
            if (maxFolderLevel == 0)
                continue;

            // Recurse this directory
            if ( (strcmp(file_name.c_str(), "..") == 0) || (strcmp(file_name.c_str(), ".") == 0) )
                continue;   // Ignore this one

            getFilesInDirectorySub(out, full_file_name, maxFolderLevel - 1);
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
