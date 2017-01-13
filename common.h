#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>


void strToUpper(std::string &string); // Transforms the pased string to uppercase.
void strToUpper(char *string);
void strToLower(std::string &string); // Transforms the pased string to lowercase.
void strToLower(char *string);

// self explanatory.
bool isValidDirectory(std::string directory);
// searches recursively for .scp files in a folder.
void getFilesInDirectorySub(std::vector<std::string> *out, std::string directory);


#endif // COMMON_H
