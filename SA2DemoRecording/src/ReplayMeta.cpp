#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "ReplayMeta.h"

ReplayMeta::ReplayMeta(const char* filename)
{
    this->upgradeBitfield = 0;
    this->character_p1 = 0;
    this->character_p2 = 0;
    this->level = 0;
    this->framecount = 0;
    if (filename != NULL)
    {
        this->read_replay_metafile(filename);
    }
}

std::vector<ReplayMeta> ReplayMeta::create_replaymeta_list(const char* path)
{
    std::vector<ReplayMeta> retList;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().string().ends_with(".ini"))
        {
            retList.emplace_back(entry.path().string().c_str());
        }
    }
    return retList;
}

std::string ReplayMeta::string()
{
    std::ostringstream retString;
    retString << "Name: " << this->name << std::endl <<
        "Description: " << this->description << std::endl <<
        "Character: " << this->character_p1 << std::endl <<
        "Upgrades: " << this->upgradeBitfield << std::endl <<
        "Level: " << this->level << std::endl <<
        "Framecount: " << this->framecount << std::endl <<
        "File: " << this->file << std::endl;

    return retString.str();
}

int ReplayMeta::read_replay_metafile(const char* inFile)
{
    std::ifstream replayFile;
    replayFile.open(inFile);
    if (!replayFile.is_open())
    {
        std::cerr << "File not found or could not be opened." << std::endl;
        return -1;
    }
    std::string key, value;
    while (std::getline(replayFile, key, '='))
    {
        key = std::regex_replace(key, std::regex("^\\s*"), "");
        if (key.compare("name") == 0)
        {
            std::getline(replayFile, value);
            this->name = value;
        }
        else if (key.compare("description") == 0)
        {
            std::getline(replayFile, value);
            this->description = value;
        }
        else if (key.compare("p1") == 0)
        {
            std::getline(replayFile, value);
            this->character_p1 = std::stoi(value);
        }
        else if (key.compare("p2") == 0)
        {
            std::getline(replayFile, value);
            this->character_p2 = std::stoi(value);
        }
        else if (key.compare("level") == 0)
        {
            std::getline(replayFile, value);
            this->level = std::stoi(value);
        }
        else if (key.compare("upgrade") == 0)
        {
            std::getline(replayFile, value);
            this->upgradeBitfield = std::stoi(value);
        }
        else if (key.compare("framecount") == 0)
        {
            std::getline(replayFile, value);
            this->framecount = std::stoi(value);
        }
        else if (key.compare("file") == 0)
        {
            std::getline(replayFile, value);
            this->file = value;
        }
        else
        {
            std::cerr << "Invalid argument." << std::endl;
            replayFile.close();
            return -1;
        }
    }
    replayFile.close();
    return 0;
}

int ReplayMeta::write_replay_metafile(const char* outFile)
{
    return write_replay_metafile(
        this->name.c_str(),
        this->description.c_str(),
        this->upgradeBitfield,
        this->character_p1,
        this->level,
        this->framecount,
        this->file.c_str(),
        outFile);
}

int ReplayMeta::write_replay_metafile(const char* name, const char* description, uint32_t upgradeBitfield, uint32_t character_p1, uint16_t level, int framecount, const char* file, const char* outFile)
{
    std::ofstream replayFile;
    replayFile.open(outFile);
    if (!replayFile.is_open())
    {
        std::cerr << "File could not be opened to write." << std::endl;
        return -1;
    }
    replayFile <<
        "name=" << name << std::endl <<
        "description=" << description << std::endl <<
        "p1=" << character_p1 << std::endl <<
        "upgrade=" << upgradeBitfield << std::endl <<
        "level=" << level << std::endl <<
        "framecount=" << framecount << std::endl <<
        "file=" << file << std::endl;
    replayFile.close();
    return 0;
}