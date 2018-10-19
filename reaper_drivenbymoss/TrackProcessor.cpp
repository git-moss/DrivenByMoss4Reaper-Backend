// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "TrackProcessor.h"
#include "OscProcessor.h"
#include "ReaperUtils.h"


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

	ReaProject *project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(path.at(0).c_str()));
	if (trackIndex < 0)
		return;
	MediaTrack *track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "scrollto") == 0)
	{
		SetOnlyTrackSelected(track);
		SetMixerScroll(track);
		this->model.deviceSelected = 0;
		return;
	}

	if (std::strcmp(cmd, "remove") == 0)
	{
		Undo_BeginBlock2(project);
		DeleteTrack(track);
		Undo_EndBlock2(project, "Delete track", 0);
		return;
	}

	if (std::strcmp(cmd, "clip") == 0)
	{
		if (path.size() < 4)
			return;
		const int clipIndex = atoi(path.at(2).c_str());
		MediaItem *item = GetTrackMediaItem(track, clipIndex);
		if (item == nullptr)
			return;

		const char *subcmd = path.at(3).c_str();

		if (std::strcmp(subcmd, "select") == 0)
		{
			Main_OnCommandEx(UNSELECT_ALL_ITEMS, 0, project);
			SetMediaItemSelected(item, true);
			UpdateTimeline();
			return;
		}

		if (std::strcmp(subcmd, "launch") == 0)
		{
			double position = GetMediaItemInfo_Value(item, "D_POSITION");
			SetEditCurPos2(project, position, true, true);
			if ((GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
			return;
		}

		if (std::strcmp(subcmd, "record") == 0)
		{
			double position = GetMediaItemInfo_Value(item, "D_POSITION");
			SetEditCurPos2(project, position, true, true);
			if ((GetPlayStateEx(project) & 4) == 0)
				CSurf_OnRecord();
			return;
		}

		if (std::strcmp(subcmd, "remove") == 0)
		{
			DeleteTrackMediaItem(track, item);
			UpdateTimeline();
			Undo_OnStateChangeEx("Delete item", UNDO_STATE_ITEMS, -1);
			return;
		}

		return;
	}
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(path.at(0).c_str()));
	if (trackIndex < 0)
		return;
	MediaTrack *track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "select") == 0)
	{
		SetOnlyTrackSelected(track);
		SetMixerScroll(track);
		this->model.deviceSelected = 0;
		return;
	}

	if (std::strcmp(cmd, "createClip") == 0)
	{
		CreateMidiClip(project, track, value);
		return;
	}

	if (std::strcmp(cmd, "noterepeat") == 0)
	{
		EnableRepeatPlugin(project, track, value > 0);
		return;
	}

	if (std::strcmp(cmd, "active") == 0)
	{
		SetIsActivated(project, value > 0);
		return;
	}

	if (std::strcmp(cmd, "solo") == 0)
	{
		CSurf_OnSoloChange(track, value);
		return;
	}

	if (std::strcmp(cmd, "mute") == 0)
	{
		CSurf_OnMuteChange(track, value);
		return;
	}

	if (std::strcmp(cmd, "recarm") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECARM", value);
		return;
	}

	if (std::strcmp(cmd, "monitor") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECMON", value > 0 ? 1 : 0);
		return;
	}

	if (std::strcmp(cmd, "autoMonitor") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECMON", value > 0 ? 2 : 0);
		return;
	}

	if (std::strcmp(cmd, "autotrim") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 0);
		return;
	}

	if (std::strcmp(cmd, "autoread") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 1);
		return;
	}

	if (std::strcmp(cmd, "autotouch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 2);
		return;
	}

	if (std::strcmp(cmd, "autowrite") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 3);
		return;
	}

	if (std::strcmp(cmd, "autolatch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 4);
		return;
	}

	Process(command, path, static_cast<double>(value));
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::string command, std::deque<std::string> &path, double value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(path.at(0).c_str()));
	if (trackIndex < 0)
		return;
	MediaTrack *track = GetTrack(project, trackIndex);
	if (!track)
		return;

	Track *trackData = this->model.GetTrack(trackIndex);
	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "volume") == 0)
	{
		// Touch not supported            
		if (path.size() == 2)
		{
			trackData->volume = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			CSurf_OnVolumeChange(track, trackData->volume, false);
		}
		return;
	}

	if (std::strcmp(cmd, "pan") == 0)
	{
		// Touch not supported            
		if (path.size() == 2)
		{
			trackData->pan = value * 2 - 1;
			CSurf_OnPanChange(track, trackData->pan, false);
		}
		return;
	}

	if (std::strcmp(cmd, "send") == 0)
	{
		const int sendIndex = atoi(path.at(2).c_str()) - 1;
		const char *subcmd = path.at(3).c_str();
		if (std::strcmp(subcmd, "volume") == 0)
		{
			trackData->sendVolume.at(sendIndex) = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			CSurf_OnSendVolumeChange(track, sendIndex, trackData->sendVolume.at(sendIndex), false);
		}
		return;
	}

	if (std::strcmp(cmd, "noterepeatlength") == 0)
	{
		SetRepeatLength(project, track, value);
		return;
	}
}


void TrackProcessor::Process(std::string command, std::deque<std::string> &path, const std::string &value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(path.at(0).c_str()));
	if (trackIndex < 0)
		return;
	MediaTrack *track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char *cmd = path.at(1).c_str();
	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfTrack(project, track, value);
		return;
	}
}


/** {@inheritDoc} */
void TrackProcessor::CreateMidiClip(ReaProject *project, MediaTrack *track, int beats)
{
	Undo_BeginBlock2(project);

	// Stop playback to update the play cursor position
	Main_OnCommandEx(1016, 0, project);

	// Disable recording on all tracks
	for (int idx = 0; idx < CountTracks(project); idx++)
		SetMediaTrackInfo_Value(GetTrack(project, idx), "I_RECARM", 0);
	// Set recording mode to midi overdub
	SetMediaTrackInfo_Value(track, "I_RECMODE", 7);
	// Enable Recording on current track
	SetMediaTrackInfo_Value(track, "I_RECARM", 1);

	// Enable Loop
	GetSetRepeatEx(project, 1);

	// Remove all current selections
	SelectAllMediaItems(project, false);

	// Create a new midi clip on the given track
	double cursorPos = GetCursorPositionEx(project);
	double bpmOut, bpiOut;
	GetProjectTimeSignature2(project, &bpmOut, &bpiOut);
	// Calculate length in seconds of n beats
	const double length = static_cast<double>(beats) * 60.0 / bpmOut;
	double end = cursorPos + length;

	MediaItem *item = CreateNewMIDIItemInProj(track, cursorPos, end, nullptr);
	if (item != nullptr)
	{
		// Set the loop to the item
		GetSet_LoopTimeRange(true, true, &cursorPos, &end, false);
		SetMediaItemSelected(item, true);

		CSurf_OnPlay();
	}

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


void TrackProcessor::SetIsActivated(ReaProject *project, bool enable)
{
	if (enable)
	{
		Undo_BeginBlock2(project);
		Main_OnCommandEx(UNLOCK_TRACK_CONTROLS, 0, project);
		Main_OnCommandEx(SET_ALL_FX_ONLINE, 0, project);
		ExecuteActionEx(project, UNMUTE_ALL_RECEIVES_ON_SELECTED_TRACKS);
		ExecuteActionEx(project, UNMUTE_ALL_SENDS_ON_SELECTED_TRACKS);
		ExecuteActionEx(project, UNBYPASS_ALL_FX_ON_SELECTED_TRACKS);
		Main_OnCommandEx(UNMUTE_TRACKS, 0, project);
		Undo_EndBlock2(project, "Enable track", 0);
	}
	else
	{
		Undo_BeginBlock2(project);
		Main_OnCommandEx(MUTE_TRACKS, 0, project);
		ExecuteActionEx(project, BYPASS_ALL_FX_ON_SELECTED_TRACKS);
		ExecuteActionEx(project, MUTE_ALL_SENDS_ON_SELECTED_TRACKS);
		ExecuteActionEx(project, MUTE_ALL_RECEIVES_ON_SELECTED_TRACKS);
		Main_OnCommandEx(SET_ALL_FX_OFFLINE, 0, project);
		Main_OnCommandEx(LOCK_TRACK_CONTROLS, 0, project);
		Undo_EndBlock2(project, "Disable track", 0);
	}
}


int TrackProcessor::GetTrackIndex(ReaProject *project, int dawTrackIndex) const
{
	int count = CountTracks(project);
	if (dawTrackIndex < 0 || dawTrackIndex >= count)
		return -1;

	int trackIndex{ 0 };
	int trackState{};
	for (int index = 0; index < count; index++)
	{
		MediaTrack *mediaTrack = GetTrack(project, index);
		if (mediaTrack == nullptr)
			continue;
		// Ignore track if hidden
		GetTrackState(mediaTrack, &trackState);
		if ((trackState & 1024) > 0)
			continue;
		if (dawTrackIndex == trackIndex)
			return index;
		trackIndex++;
	}
	return -1;
}