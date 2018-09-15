// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <vector>


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

	int isSelected;
	int mute;
	int solo;
	int recArmed;
	int isActive;
	int monitor;
	int autoMonitor;
	
	std::string color;

	double volume;
	std::string volumeStr;
	double pan;
	std::string panStr;
	double vuLeft;
	double vuRight;
	int autoMode;

	int repeatActive;
	int repeatNoteLength;

	std::vector<std::string> sendName;
	std::vector<double> sendVolume;
	std::vector<std::string> sendVolumeStr;


	Track(const int sendBankSize);
	virtual ~Track();

	void CollectData(std::stringstream &ss, ReaProject *project, MediaTrack *track, int trackIndex, const bool &dump);

	int GetTrackLockState(MediaTrack *track);
};
