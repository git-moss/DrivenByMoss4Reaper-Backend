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

	int trackExists;
	int trackNumber;
	int trackDepth;
	std::string trackName;
	std::string trackType;

	int trackSelected;
	int trackMute;
	int trackSolo;
	int trackRecArmed;
	int trackActive;
	int trackMonitor;
	int trackAutoMonitor;
	
	std::string trackColor;

	double trackVolume;
	std::string trackVolumeStr;
	double trackPan;
	std::string trackPanStr;
	double trackVULeft;
	double trackVURight;
	int trackAutoMode;

	int trackRepeatActive;
	int trackRepeatNoteLength;

	std::vector<std::string> trackSendName;
	std::vector<double> trackSendVolume;
	std::vector<std::string> trackSendVolumeStr;


	Track(const int sendBankSize);
	virtual ~Track();

	void CollectData(std::stringstream &ss, ReaProject *project, int trackIndex, int trackCount, const bool &dump);

	int GetTrackLockState(MediaTrack *track);
};
