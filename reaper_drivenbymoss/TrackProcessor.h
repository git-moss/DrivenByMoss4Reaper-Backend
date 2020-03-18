// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_TRACKPROCESSOR_H_
#define _DBM_TRACKPROCESSOR_H_

#include <string>
#include "OscProcessor.h"


/**
 * Processes all commands related to tracks.
 */
class TrackProcessor : public OscProcessor
{
public:
	TrackProcessor(Model &model);

	void Process(std::deque<std::string> &path) noexcept override;
	void Process(std::deque<std::string> &path, int value) noexcept override;
	void Process(std::deque<std::string> &path, double value) noexcept override;
	void Process(std::deque<std::string> &path, const std::string &value) noexcept override;

private:
	void CreateMidiClip(ReaProject *project, MediaTrack *track, int beats) noexcept;
	void RecordMidiClip(ReaProject* project, MediaTrack* track) noexcept;

	void SetColorOfTrack(ReaProject *project, MediaTrack *track, std::string value);
	void SetIsActivated(ReaProject *project, bool enable);
	void DeleteAllAutomationEnvelopes(ReaProject *project, MediaTrack *track) noexcept;
	int GetTrackIndex(ReaProject *project, int dawTrackIndex) const noexcept;
};

#endif /* _DBM_TRACKPROCESSOR_H_ */