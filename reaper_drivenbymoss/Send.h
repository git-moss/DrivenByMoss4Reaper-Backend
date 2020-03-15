// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_SEND_H_
#define _DBM_SEND_H_

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

	void CollectData(std::stringstream& ss, ReaProject* project, MediaTrack* track, int sendIndex, const std::string& trackAddress, const bool& dump);

private:
	double GetSendVolume(MediaTrack* track, int sendCounter, double position) const;
};

#endif /* _DBM_SEND_H_ */