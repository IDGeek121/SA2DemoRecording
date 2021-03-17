#pragma once
#ifndef REPLAYMETA_H
#define REPLAYMETA_H
#include <string>
#include <vector>

class ReplayMeta {
public:
    std::string author;
    std::string file;
    uint32_t upgradeBitfield;
    uint32_t character_p1;
    uint32_t character_p2;
    int framecount; // For RNG Purposes
    bool restart; // true if replay should start with a restart. false if replay starts normally.
    uint16_t level;

    ReplayMeta()
        : author(), upgradeBitfield(0), character_p1(0), character_p2(0), framecount(0), restart(false), level(0), file()
    {
    }
    ReplayMeta(const char* filename);
    /* Takes in a filename and sets the ReplayMeta fields. Returns -1 if there's an error. */
    int read_replay_metafile(const char* filename);
    /* Writes a replaymeta file to disk. */
    int write_replay_metafile(const char* outFile);
    /* Looks for all .ini files in a path and parses them as replaymeta files. */
    static std::vector<ReplayMeta> create_replaymeta_list(const char* path);
    /* Writes a replaymeta file to disk. */
    static int write_replay_metafile(
        const char* author,
        uint32_t upgradeBitfield,
        uint32_t character_p1,
        uint16_t level,
        int framecount,
        bool restart,
        const char* file,
        const char* outFile
    );
    /* Outputs the replaymeta to a printable string. */
    std::string string();
};

#endif