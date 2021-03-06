#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "ReplayMeta.h"

ReplayMeta::ReplayMeta(const char* filename) {
    this->upgradeBitfield = 0;
    this->character_p1 = 0;
    this->character_p2 = 0;
    this->level = 0;
    this->framecount = 0;
    this->restart = false;
    if (filename != nullptr)
        this->read_replay_metafile(filename);
}

std::vector<ReplayMeta> ReplayMeta::create_replaymeta_list(const char* path) {
    std::vector<ReplayMeta> retList;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.path().string().ends_with(".ini"))
            retList.emplace_back(entry.path().string().c_str());
    }
    return retList;
}

std::string ReplayMeta::string() {
    std::ostringstream retString;
    retString << "Author: " << this->author << std::endl <<
        "Character: " << this->character_p1 << std::endl <<
        "Upgrades: " << this->upgradeBitfield << std::endl <<
        "Level: " << this->level << std::endl <<
        "Framecount: " << this->framecount << std::endl <<
        "Restart: " << std::boolalpha << this->restart << std::endl <<
        "File: " << this->file << std::endl;

    return retString.str();
}

int ReplayMeta::read_replay_metafile(const char* inFile) {
    std::ifstream replayFile;
    replayFile.open(inFile);
    if (!replayFile.is_open()) {
        std::cerr << "File not found or could not be opened." << std::endl;
        return -1;
    }
    std::string key, value;
    while (std::getline(replayFile, key, '='))
    {
        key = std::regex_replace(key, std::regex("^\\s*"), "");        
        if (key.compare("author") == 0) {
            std::getline(replayFile, value);
            this->author = value;
        }
        else if (key.compare("p1") == 0) {
            std::getline(replayFile, value);
            this->character_p1 = std::stoi(value);
        }
        else if (key.compare("p2") == 0) {
            std::getline(replayFile, value);
            this->character_p2 = std::stoi(value);
        }
        else if (key.compare("level") == 0) {
            std::getline(replayFile, value);
            this->level = std::stoi(value);
        }
        else if (key.compare("upgrade") == 0) {
            std::getline(replayFile, value);
            this->upgradeBitfield = std::stoi(value);
        }
        else if (key.compare("framecount") == 0) {
            std::getline(replayFile, value);
            this->framecount = std::stoi(value);
        }
        else if (key.compare("restart") == 0) {
            std::getline(replayFile, value);
            this->restart = std::stoi(value);
        }
        else if (key.compare("file") == 0) {
            std::getline(replayFile, value);
            this->file = value;
        }
        // Ignore these two
        else if (key.compare("name") == 0) {
            std::getline(replayFile, value);
        }
        else if (key.compare("description") == 0) {
            std::getline(replayFile, value);
        }
        else {
            std::cerr << "Invalid argument \"" << key << "\" found." << std::endl;
            std::getline(replayFile, value);
        }
    }
    replayFile.close();
    return 0;
}

int ReplayMeta::write_replay_metafile(const char* outFile) {
    return write_replay_metafile(
        this->author.c_str(),
        this->upgradeBitfield,
        this->character_p1,
        this->level,
        this->framecount,
        this->restart,
        this->file.c_str(),
        outFile);
}

int ReplayMeta::write_replay_metafile(const char* author, uint32_t upgradeBitfield, uint32_t character_p1, uint16_t level, int framecount, bool restart, const char* file, const char* outFile) {
    std::ofstream replayFile;
    replayFile.open(outFile);
    if (!replayFile.is_open()) {
        std::cerr << "File could not be opened to write." << std::endl;
        return -1;
    }
    replayFile <<
        "author=" << author << std::endl <<
        "p1=" << character_p1 << std::endl <<
        "upgrade=" << upgradeBitfield << std::endl <<
        "level=" << level << std::endl <<
        "framecount=" << framecount << std::endl <<
        "restart=" << std::noboolalpha << restart << std::endl <<
        "file=" << file << std::endl;
    replayFile.close();
    return 0;
}