// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
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
	int enabled;
	std::string name;
	double volume;
	std::string volumeStr;
	std::string color;

	Send() noexcept;
	Send(const Send&) = delete;
	Send& operator=(const Send&) = delete;
	Send(Send&&) = delete;
	Send& operator=(Send&&) = delete;
	virtual ~Send();

	void CollectData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, int sendIndex, const std::string& trackAddress, const bool& dump);

private:
	double GetSendVolume(MediaTrack* track, int sendCounter, double position) const noexcept;
};

#endif /* _DBM_SEND_H_ */
