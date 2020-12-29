#pragma once
#ifndef REPLAYMETA_H
#define REPLAYMETA_H

class ReplayMeta {
public:
	std::string name;
	std::string description;
	std::string file;
	uint32_t upgradeBitfield;
	uint32_t character_p1;
	uint32_t character_p2;
	int framecount; // For RNG Purposes
	uint16_t level;

public:
	ReplayMeta()
		: name(), description(), upgradeBitfield(0), character_p1(0), character_p2(0), framecount(0), level(0), file()
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
	static int write_replay_metafile(const char* name,
		const char* description,
		uint32_t upgradeBitfield,
		uint32_t character_p1,
		uint16_t level,
		int framecount,
		const char* file,
		const char* outFile
	);
	/* Outputs the replaymeta to a printable string. */
	std::string string();
};

#endif