// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
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
void NoteRepeatProcessor::Process(std::deque<std::string>& path, int value) noexcept
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
void NoteRepeatProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack(project, 0);
	if (!track)
		return;

	const char* cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "rate") == 0)
	{
		this->SetParameter(project, track, NoteRepeatProcessor::MIDI_ARP_PARAM_RATE, value);
		return;
	}

	if (std::strcmp(cmd, "notelength") == 0)
	{
		this->SetParameter(project, track, NoteRepeatProcessor::MIDI_ARP_PARAM_NOTE_LENGTH, value);
		return;
	}

	if (std::strcmp(cmd, "mode") == 0)
	{
		this->SetParameter(project, track, NoteRepeatProcessor::MIDI_ARP_PARAM_MODE, value);
		return;
	}

	if (std::strcmp(cmd, "velocity") == 0)
	{
		this->SetParameter(project, track, NoteRepeatProcessor::MIDI_ARP_PARAM_VELOCITY, value == 0 ? 127 : 0);
		return;
	}
}


void NoteRepeatProcessor::EnableRepeatPlugin(ReaProject* project, MediaTrack* track, bool enable) const noexcept
{
	const int position = TrackFX_AddByName(track, NoteRepeatProcessor::MIDI_ARP_PLUGIN, 1, 1);
	if (position > -1)
	{
		// Note: 0x1000000 selects plugins on the record input FX chain
		TrackFX_SetEnabled(track, 0x1000000 + position, enable);
	}
}


void NoteRepeatProcessor::SetParameter(ReaProject* project, MediaTrack* track, int parameterIndex, double value) const noexcept
{
	const int position = TrackFX_AddByName(track, NoteRepeatProcessor::MIDI_ARP_PLUGIN, 1, 1);
	if (position > -1)
	{
		// Note: 0x1000000 selects plugins on the record input FX chain
		const int inputPosition = 0x1000000 + position;

		double minVal{}, maxVal{};
		TrackFX_GetParam(track, inputPosition, parameterIndex, &minVal, &maxVal);

		const double range = maxVal - minVal;
		TrackFX_SetParamNormalized(track, inputPosition, parameterIndex, value / range);
	}
}
