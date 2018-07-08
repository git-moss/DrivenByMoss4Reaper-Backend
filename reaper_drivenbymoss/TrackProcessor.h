// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to tracks.
 */
class TrackProcessor : public OscProcessor
{
public:
	TrackProcessor(Model &model);

	void Process(std::string command, std::deque<std::string> &path) override;
	void Process(std::string command, std::deque<std::string> &path, int value) override;
	void Process(std::string command, std::deque<std::string> &path, double value) override;
	void Process(std::string command, std::deque<std::string> &path, const std::string &value) override;

private:
	void CreateMidiClip(ReaProject *project, MediaTrack *track, int beats);
	void EnableRepeatPlugin(ReaProject *project, MediaTrack *track, bool enable);
	void SetRepeatLength(ReaProject *project, MediaTrack *track, double resolution);
	void SetColorOfTrack(ReaProject *project, MediaTrack *track, std::string value);
};
