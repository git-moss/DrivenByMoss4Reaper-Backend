// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


class TrackProcessor : public OscProcessor
{
public:
	TrackProcessor(Model *model);

	virtual void Process(std::string command, std::deque<std::string> &path);
	virtual void Process(std::string command, std::deque<std::string> &path, int value);
	virtual void Process(std::string command, std::deque<std::string> &path, double value);
	virtual void Process(std::string command, std::deque<std::string> &path, std::string value);

private:
	void CreateMidiClip(ReaProject *project, MediaTrack *track, int beats);
	void EnableRepeatPlugin(ReaProject *project, MediaTrack *track, bool enable);
	void SetRepeatLength(ReaProject *project, MediaTrack *track, double resolution);
	void SetColorOfTrack(ReaProject *project, MediaTrack *track, std::string value);
};
