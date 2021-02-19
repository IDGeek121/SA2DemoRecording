#pragma once
class Config {
public:
    std::string author;

    Config() : author() {}
    Config(const char* filename);
};