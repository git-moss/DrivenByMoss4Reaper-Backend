// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_TRACK_H_
#define _DBM_TRACK_H_

#include <mutex>
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
	static const std::regex LOCK_PATTERN;
	static const std::regex INPUT_QUANTIZE_PATTERN;

	int exists{ 0 };
	int number{ 0 };
	int depth{ 0 };
	std::string name;
	std::string type;
	int isGroupExpanded{ 1 };

	int isSelected{ 0 };
	int mute{ 0 };
	int solo{ 0 };
	int recArmed{ 0 };
	int monitor{ 0 };
	int autoMonitor{ 0 };
	int overdub{ 0 };

	std::string color;

	double volume{ 0.0 };
	std::string volumeStr;
	bool isVolumeTouch{ false };

	double pan{ 0.0 };
	std::string panStr;
	bool isPanTouch{ false };

	double vu{ 0.0 };
	double vuLeft{ 0.0 };
	double vuRight{ 0.0 };
	int autoMode{ 0 };


	Track() noexcept;

	void CollectData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, int trackIndex, const bool& slowUpdate, const bool& dump);

	std::unique_ptr<Send>& GetSend(const int index);

	double GetVolume(MediaTrack* track, double position) const noexcept;
	double GetPan(MediaTrack* track, double position) const noexcept;
	int GetMute(MediaTrack* track, double position, int trackState) const noexcept;

private:
	int sendCount{ 0 };
	std::vector<std::unique_ptr<Send>> sends;
	std::mutex sendlock;
};

#endif /* _DBM_TRACK_H_ */