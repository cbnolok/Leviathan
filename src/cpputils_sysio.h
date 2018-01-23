#ifndef SYSIO_H
#define SYSIO_H

#include <string>
#include <vector>


void standardizePath(std::string &s);   // Change back slash to forward slash and add (if not present) trailing forward slash

// self explanatory.
bool isValidFile(const std::string& filePath);
bool isValidDirectory(const std::string& directoryPath);

// searches recursively for .scp files in a folder.
void getFilesInDirectorySub(std::vector<std::string> *out, std::string directory);


#endif // SYSIO_H
