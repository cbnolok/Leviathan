#ifndef SYSIO_H
#define SYSIO_H

#include <string>
#include <string_view>
#include <vector>

#ifdef QT_CORE_LIB
#include <QStringView>
#endif


#ifdef QT_CORE_LIB
QString standardizePath(QStringView s);
#endif
std::string standardizePath(std::string_view s);   // Change back slash to forward slash and add (if not present) trailing forward slash

// self explanatory.
bool isValidFile(std::string_view filePath);
bool isValidDirectory(std::string_view directoryPath);

size_t countDirLevels(std::string_view str);
bool comparatorDirLevels(std::string_view lhs, std::string_view rhs);

std::string getDirectoryFromString(std::string_view path);

// searches recursively for .scp files in a folder.
void getFilesInDirectorySub(std::vector<std::string> *out, std::string path, int maxFolderLevel);


#endif // SYSIO_H
