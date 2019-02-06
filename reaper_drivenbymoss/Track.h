// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <vector>

#include "ReaperUtils.h"


/**
 * Encapsulates Reaper C functions for tracks.
 */
class Track
{
public:
	int sendBankSize;

	int exists{ 0 };
	int number{ 0 };
	int depth{ 0 };
	std::string name{ "" };
	std::string type;

	int isSelected{ 0 };
	int mute{ 0 };
	int solo{ 0 };
	int recArmed{ 0 };
	int isActive{ 0 };
	int monitor{ 0 };
	int autoMonitor{ 0 };

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

	std::vector<std::string> sendName;
	std::vector<double> sendVolume;
	std::vector<std::string> sendVolumeStr;


	Track(const int sendBankSize);
	virtual ~Track();

	void CollectData(std::stringstream &ss, ReaProject *project, MediaTrack *track, int trackIndex, const bool &dump);

	int GetTrackLockState(MediaTrack *track);
};
