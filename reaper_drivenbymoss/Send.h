// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "ReaperUtils.h"


/**
 * Encapsulates Reaper C functions for sends.
 */
class Send
{
public:
	int position{ 0 };
	std::string name;
	double volume{ 0 };
	std::string volumeStr;

	Send();
	virtual ~Send();

	void CollectData(std::stringstream& ss, ReaProject* project, MediaTrack* track, int sendIndex, std::string& trackAddress, const bool& dump);

private:
	double GetSendVolume(MediaTrack* track, int sendCounter, double position) const;
};
