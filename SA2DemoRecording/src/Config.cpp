#include <string>

#include "SA2ModLoader.h"
#include "IniFile.hpp"
#include "Config.h"

Config::Config(const char* filename) {
    IniFile config = IniFile(filename);

    this->author = config.getString("set", "author", "n/a");
}