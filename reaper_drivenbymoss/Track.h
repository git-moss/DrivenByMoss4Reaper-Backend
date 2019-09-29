// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <vector>
#include <regex>

#include "ReaperUtils.h"
#include "Send.h"


/**
 * Encapsulates Reaper C functions for tracks.
 */
class Track
{
public:
	static const int NAME_LENGTH{ 20 };
	static const int CHUNK_LENGTH{ 4096 };
	static const std::regex LOCK_PATTERN;
	static const std::regex INPUT_QUANTIZE_PATTERN;

	int exists{ 0 };
	int number{ 0 };
	int depth{ 0 };
	std::string name{ "" };
	std::string type;

	int isSelected{ 0 };
	int mute{ 0 };
	int solo{ 0 };
	int recArmed{ 0 };
	int monitor{ 0 };
	int autoMonitor{ 0 };

	int isActive{ 0 };
	int inQuantEnabled{ 0 };
	int inQuantLengthEnabled{ 0 };
	double inQuantResolution{ 0.25 };

	std::string color;

	double volume{ 0.0 };
	std::string volumeStr;
	double pan{ 0.0 };
	std::string panStr;
	double vuLeft{ 0.0 };
	double vuRight{ 0.0 };
	int autoMode{ 0 };

	int repeatActive{ 0 };
	double repeatNoteLength{ 0 };


	Track();
	virtual ~Track();

	void CollectData(std::stringstream& ss, ReaProject* project, MediaTrack* track, int trackIndex, const bool &slowUpdate, const bool& dump);

	Send* GetSend(const int index);

	double GetVolume(MediaTrack* track, double position) const;
	double GetPan(MediaTrack* track, double position) const;
	int GetMute(MediaTrack* track, double position, int trackState) const;

private:
	int sendCount{ 0 };
	std::vector<Send*> sends;
	std::mutex sendlock;

	double GetValue(MediaTrack* track, double position, const char* envelopeName, const char* infoName) const;
	int GetTrackLockState(char* chunk) const;
	void ParseInputQuantize(std::stringstream& ss, std::string& trackAddress, const bool& dump, char* chunk);
};
