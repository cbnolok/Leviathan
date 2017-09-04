#ifndef SYSIO_H
#define SYSIO_H

#include <string>
#include <vector>


// self explanatory.
bool isValidFile(std::string filePath);
bool isValidDirectory(std::string directoryPath);

// searches recursively for .scp files in a folder.
void getFilesInDirectorySub(std::vector<std::string> *out, std::string directory);


#endif // SYSIO_H
