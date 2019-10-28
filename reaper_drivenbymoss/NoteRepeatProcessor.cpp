// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <cstring>
#include <sstream>

#include "NoteRepeatProcessor.h"
#include "OscProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
NoteRepeatProcessor::NoteRepeatProcessor(Model& aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void NoteRepeatProcessor::Process(std::deque<std::string>& path, int value)
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack(project, 0);
	if (!track)
		return;

	const char* cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "active") == 0)
	{
		EnableRepeatPlugin(project, track, value > 0);
		return;
	}

	Process(path, static_cast<double>(value));
}


/** {@inheritDoc} */
void NoteRepeatProcessor::Process(std::deque<std::string>& path, double value)
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack(project, 0);
	if (!track)
		return;

	const char* cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "length") == 0)
	{
		SetRepeatLength(project, track, value);
		return;
	}
}


void NoteRepeatProcessor::EnableRepeatPlugin(ReaProject* project, MediaTrack* track, bool enable)
{
	// Get or insert note midi repeat plugin
	Undo_BeginBlock2(project);
	const int position = TrackFX_AddByName(track, "midi_note_repeater", 1, 1);
	if (position > -1)
	{
		// Note: 0x1000000 selects plugins on the record input FX chain
		TrackFX_SetEnabled(track, 0x1000000 + position, enable);
	}
	Undo_EndBlock2(project, "Dis-/enable note repeat (inserts plugin)", 0);
}


void NoteRepeatProcessor::SetRepeatLength(ReaProject* project, MediaTrack* track, double resolution)
{
	Undo_BeginBlock2(project);
	const int position = TrackFX_AddByName(track, "midi_note_repeater", 1, 1);
	if (position > -1)
	{
		// Note: 0x1000000 selects plugins on the record input FX chain
		TrackFX_SetParam(track, 0x1000000 + position, 0, resolution);
	}
	Undo_EndBlock2(project, "Set note repeat length", 0);
}
