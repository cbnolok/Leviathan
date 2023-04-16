#include "sysio.h"
#include <sys/stat.h>
#include <cassert>
#include <algorithm>    // for std::replace

#ifdef _WIN32
    #include <filesystem>
#else
    #include <dirent.h>     // To search files inside a directory
    #include <cstring>
#endif

#ifdef QT_CORE_LIB
    #include <QString>
#endif


inline static constexpr char kStdDirDelim = '/';


#ifdef QT_CORE_LIB
QString standardizePath(QStringView s)
{
    QString ret(s.toString());
    if (ret.isEmpty())
        return ret;

    std::replace(ret.begin(), ret.end(), '\\', kStdDirDelim);
    if (isValidDirectory(ret.toStdString()))
    {
        if (ret[ret.length() - 1] != kStdDirDelim)
            ret += kStdDirDelim;
    }

    return ret;
}
#endif

std::string standardizePath(std::string_view s)
{
    std::string ret(s);
    if (s.empty())
        return ret;

    std::replace(ret.begin(), ret.end(), '\\', kStdDirDelim);
    if (isValidDirectory(s))
    {
        if (ret[ret.length() - 1] != kStdDirDelim)
            ret += kStdDirDelim;
    }

    return ret;
}

bool isValidFile(std::string_view filePath)
{
    //return std::filesystem::is_regular_file(filePath);  // Can't use this, since it was added to MacOS only since version 10.15...
    struct stat sb;
    return (stat(filePath.data(), &sb) == 0);
}

bool isValidDirectory(std::string_view directoryPath)
{
    //return std::filesystem::is_directory(directoryPath); // Can't use this, since it was added to MacOS only since version 10.15...
    struct stat sb;
    return (stat(directoryPath.data(), &sb) == 0) && ((sb.st_mode & S_IFDIR) == S_IFDIR);
}


size_t countDirLevels(std::string_view str)
{
    return std::count(str.cbegin(), str.cend(), kStdDirDelim);
}

bool comparatorDirLevels(std::string_view lhs, std::string_view rhs)
{
    return (countDirLevels(lhs) < countDirLevels(rhs));
}

std::string getDirectoryFromString(std::string_view path)
{
    const std::string ret(standardizePath(path));
    if (path.empty())
        return ret;
    return ret.substr(0, ret.find_last_of(kStdDirDelim));
}


void getFilesRecurseDirectories(std::vector<std::string> *out, std::string path, int maxFolderLevel)
{
    // This function adds to the list, recursively, files inside folders.
    // TODO: right now doesn't get saves and .ini.
    assert(!path.empty());
    path = standardizePath(path);

#ifdef _WIN32
    namespace fs = std::filesystem;

    if (!fs::is_directory(path))
    {
        out->emplace_back(path);
        return;
    }

    std::string entry_name;
    std::string entry_path;
    for (auto const& entry : fs::directory_iterator(path))
    {
        entry_path = standardizePath(entry.path().string());
        if (fs::is_directory(entry_path))
        {
            // Recurse this directory
            if (maxFolderLevel != 0)
            {
                getFilesRecurseDirectories(out, entry_path, maxFolderLevel - 1);
                out->emplace_back(entry_path);
            }
            continue;
        }

        entry_name = entry.path().filename().string();
        if (entry_name[0] == '.')
            continue;
        if (entry_name.length() <= 4)
            continue;
        if (strcmp(entry_name.c_str() + entry_name.length() - 4, ".scp") != 0)
            continue;   // we look only for .scp files.

        out->emplace_back(entry_path);
    }

#else
    DIR *dir = opendir(path.c_str());
    std::string file_name;
    std::string full_file_name;
    while (class dirent *ent = readdir(dir))
    {
        file_name = ent->d_name;
        full_file_name = path + kStdDirDelim + file_name;

        class stat st;
        if (stat(full_file_name.c_str(), &st) == -1)
            continue;
        const bool is_directory = (st.st_mode & S_IFDIR) == S_IFDIR;
        if (is_directory)
        {
            if (maxFolderLevel == 0)
                continue;

            // Recurse this directory
            if ( (strcmp(file_name.c_str(), "..") == 0) || (strcmp(file_name.c_str(), ".") == 0) )
                continue;   // Ignore this one

            getFilesRecurseDirectories(out, full_file_name, maxFolderLevel - 1);
            continue;
        }
        else if (file_name[0] == '.')
            continue;
        else if (file_name.length() <= 4)
            continue;
        else if (strcmp(file_name.c_str() + file_name.length() - 4, ".scp") != 0)
            continue;   // we look only for .scp files.

        out->emplace_back(full_file_name);
    }
    closedir(dir);
#endif
}
