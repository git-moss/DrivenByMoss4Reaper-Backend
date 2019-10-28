// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include "OscProcessor.h"


/**
 * Processes all commands related to note repeat.
 */
class NoteRepeatProcessor : public OscProcessor
{
public:
	NoteRepeatProcessor(Model &model);

	void Process(std::deque<std::string> &path, int value) override;
	void Process(std::deque<std::string> &path, double value) override;

private:
	void EnableRepeatPlugin(ReaProject *project, MediaTrack *track, bool enable);
	void SetRepeatLength(ReaProject *project, MediaTrack *track, double resolution);
};
