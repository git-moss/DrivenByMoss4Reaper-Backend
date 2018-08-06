// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "TrackProcessor.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
TrackProcessor::TrackProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::string command, std::deque<std::string> &path)
{
	if (path.size() < 2)
		return;
	const char *part = path.at(0).c_str();

	if (std::strcmp(part, "bank") == 0)
	{
		const char *cmd = path.at(1).c_str();
		if (std::strcmp(cmd, "+") == 0)
		{
			if (this->model.trackBankOffset < this->model.trackCount)
				this->model.trackBankOffset += this->model.trackBankSize;
		}
		else if (std::strcmp(cmd, "-") == 0)
		{
			if (this->model.trackBankOffset > 0)
				this->model.trackBankOffset -= this->model.trackBankSize;
		}
		return;
	}

	if (std::strcmp(part, "scrollto") == 0)
	{
		const int position = atoi(path.at(1).c_str());

		ReaProject *project = this->model.GetProject();
		if (position >= CountSelectedTracks(project))
			return;

		MediaTrack *track = GetTrack(project, position);
		SetOnlyTrackSelected(track);
		SetMixerScroll(track);
		this->model.deviceSelected = 0;
		this->model.deviceParamBankSelectedTemp = 0;
		return;
	}

	ReaProject *project = this->model.GetProject();
	const int index = atoi(path.at(0).c_str()) - 1;
	if (index < 0)
		return;
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + index);

	const char *cmd = path.at(1).c_str();
	if (std::strcmp(cmd, "remove") == 0)
	{
		if (track)
		{
			// UI operations must be executed on the main tread
			this->model.AddFunction([=]()
			{
				Undo_BeginBlock2(project);
				DeleteTrack(track);
				Undo_EndBlock2(project, "Delete track", 0);
			});
		}
		return;
	}
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = this->model.GetProject();
	const int index = atoi(path.at(0).c_str()) - 1;
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + index);
	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "select") == 0)
	{
		SetOnlyTrackSelected(track);
		SetMixerScroll(track);
		this->model.deviceSelected = 0;
		this->model.deviceParamBankSelectedTemp = 0;
	}
	else if (std::strcmp(cmd, "createClip") == 0)
	{
		CreateMidiClip(project, track, value);
	}
	else if (std::strcmp(cmd, "noterepeat") == 0)
	{
		EnableRepeatPlugin(project, track, value > 0);
	}
	else if (std::strcmp(cmd, "solo") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_SOLO", value);
	}
	else if (std::strcmp(cmd, "mute") == 0)
	{
		SetMediaTrackInfo_Value(track, "B_MUTE", value);
	}
	else if (std::strcmp(cmd, "recarm") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECARM", value);
	}
	else if (std::strcmp(cmd, "monitor") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECMON", value > 0 ? 1 : 0);
	}
	else if (std::strcmp(cmd, "autoMonitor") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECMON", value > 0 ? 2 : 0);
	}
	else if (std::strcmp(cmd, "autotrim") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 0);
	}
	else if (std::strcmp(cmd, "autoread") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 1);
	}
	else if (std::strcmp(cmd, "autotouch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 2);
	}
	else if (std::strcmp(cmd, "autowrite") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 3);
	}
	else if (std::strcmp(cmd, "autolatch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 4);
	}
	else
		Process(command, path, static_cast<double>(value));
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::string command, std::deque<std::string> &path, double value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = this->model.GetProject();
	const int index = atoi(path.at(0).c_str()) - 1;
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + index);
	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "volume") == 0)
	{
		// Touch not supported            
		if (path.size() == 2)
		{
			this->model.trackVolume.at(index) = this->model.DBToValue(SLIDER2DB(value * 1000.0));
			SetMediaTrackInfo_Value(track, "D_VOL", this->model.trackVolume.at(index));
		}
	}
	else if (std::strcmp(cmd, "pan") == 0)
	{
		// Touch not supported            
		if (path.size() == 2)
		{
			this->model.trackPan.at(index) = value * 2 - 1;
			SetMediaTrackInfo_Value(track, "D_PAN", this->model.trackPan.at(index));
		}
	}
	else if (std::strcmp(cmd, "send") == 0)
	{
		const int sendIndex = atoi(path.at(2).c_str()) - 1;
		const char *subcmd = path.at(3).c_str();
		if (std::strcmp(subcmd, "volume") == 0)
		{
			this->model.trackSendVolume.at(index).at(sendIndex) = this->model.DBToValue(SLIDER2DB(value * 1000.0));
			SetTrackSendInfo_Value(track, 0, sendIndex, "D_VOL", this->model.trackSendVolume.at(index).at(sendIndex));
		}
	}
	else if (std::strcmp(cmd, "noterepeatlength") == 0)
	{
		SetRepeatLength(project, track, value);
	}
}


void TrackProcessor::Process(std::string command, std::deque<std::string> &path, const std::string &value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = this->model.GetProject();
	const int index = atoi(path.at(0).c_str()) - 1;
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + index);
	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfTrack(project, track, value);
	}
}


/** {@inheritDoc} */
void TrackProcessor::CreateMidiClip(ReaProject *project, MediaTrack *track, int beats)
{
	const int selectedTrackCount = CountSelectedTracks(project);
	if (selectedTrackCount > 0)
	{
		Undo_BeginBlock2(project);

		// Stop playback to update the play cursor position
		Main_OnCommandEx(1016, 0, project);

		// Disable recording on all tracks
		const int trackCount = CountTracks(project);
		for (int idx = 0; idx < trackCount; idx++)
		{
			track = GetTrack(project, idx);
			SetMediaTrackInfo_Value(track, "I_RECARM", 0);
		}
	}

	// Create a new midi clip on the given track
	const double cursorPos = GetCursorPositionEx(project);
	double bpmOut, bpiOut;
	GetProjectTimeSignature2(project, &bpmOut, &bpiOut);
	// Calculate length in seconds of n beats
	const double length = static_cast<double>(beats) * 60.0 / bpmOut;
	MediaItem *item = CreateNewMIDIItemInProj(track, cursorPos, cursorPos + length, 0);

	// Remove all current selections
	SelectAllMediaItems(project, 0);
	SetMediaItemSelected(item, 1);

	// Set time selection to seleted items
	Main_OnCommandEx(40290, 0, project);

	// Enable Loop
	GetSetRepeatEx(project, 1);

	// Set recording mode to midi overdub
	SetMediaTrackInfo_Value(track, "I_RECMODE", 7);

	// Enable Recording
	SetMediaTrackInfo_Value(track, "I_RECARM", 1);

	// Record
	Main_OnCommandEx(1013, 0, project);

	Undo_EndBlock2(project, "Create Midi Clip and Record", 0);
}


void TrackProcessor::EnableRepeatPlugin(ReaProject *project, MediaTrack *track, bool enable)
{
	if (track == nullptr)
		return;

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


void TrackProcessor::SetRepeatLength(ReaProject *project, MediaTrack *track, double resolution)
{
	if (track == nullptr)
		return;

	Undo_BeginBlock2(project);
	const int position = TrackFX_AddByName(track, "midi_note_repeater", 1, 0);
	if (position > -1)
	{
		// Note: 0x1000000 selects plugins on the record input FX chain
		TrackFX_SetParam(track, 0x1000000 + position, 0, resolution);
	}
	Undo_EndBlock2(project, "Set note repeat length", 0);
}


void TrackProcessor::SetColorOfTrack(ReaProject *project, MediaTrack *track, std::string value)
{
	if (track == nullptr)
		return;

	std::cmatch result;
	if (!std::regex_search(value.c_str(), result, colorPattern))
		return;
	int red = std::atoi(result.str(1).c_str());
	int green = std::atoi(result.str(2).c_str());
	int blue = std::atoi(result.str(3).c_str());

	Undo_BeginBlock2(project);
	SetTrackColor(track, ColorToNative(red, green, blue));
	Undo_EndBlock2(project, "Set track color", 0);
}
