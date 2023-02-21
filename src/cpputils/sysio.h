#ifndef SYSIO_H
#define SYSIO_H

#include <string>
#include <vector>

#ifdef QT_CORE_LIB
    class QString;
#endif


#ifdef QT_CORE_LIB
QString & standardizePath(QString &s);
#endif
std::string & standardizePath(std::string &s);   // Change back slash to forward slash and add (if not present) trailing forward slash

// self explanatory.
bool isValidFile(const std::string& filePath);
bool isValidDirectory(const std::string& directoryPath);

size_t countDirLevels(std::string const& str);
bool comparatorDirLevels(std::string const& lhs, std::string const& rhs);

std::string getDirectoryFromString(std::string const& str);

// searches recursively for .scp files in a folder.
void getFilesInDirectorySub(std::vector<std::string> *out, std::string path);


#endif // SYSIO_H
