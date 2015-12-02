#ifndef CLI_H_
#define CLI_H_

#include <iostream>
#include <string>
#include <sys/stat.h>

int initCli();
std::string getImgPath(std::string);
inline bool fileExists(const std::string& name);

#endif // !CLI_H_