// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <cstring>
#include <sstream>

#include "CodeAnalysis.h"
#include "TrackProcessor.h"
#include "OscProcessor.h"
#include "ReaperUtils.h"
#include "ReaDebug.h"
#include "StringUtils.h"

/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
TrackProcessor::TrackProcessor(Model& aModel) noexcept : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::deque<std::string>& path) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	const char* param = SafeGet(path, 0);
	const int dawIndex = atoi(param);
	int trackIndex = GetTrackIndex(project, dawIndex);
	MediaTrack* track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char* cmd = SafeGet(path, 1);

	if (std::strcmp(cmd, "scrollto") == 0)
	{
		SetOnlyTrackSelected(track);
		ScrollTrackIntoView(track);
		this->model.deviceSelected = 0;
		return;
	}

	if (std::strcmp(cmd, "remove") == 0)
	{
		PreventUIRefresh(1);
		Undo_BeginBlock2(project);
		DeleteTrack(track);
		// Make sure that a track is selected
		if (GetSelectedTrack2(project, 0, true) == nullptr)
		{
			if (trackIndex >= CountTracks(project))
				trackIndex = trackIndex - 1;
			if (trackIndex >= 0)
			{
				track = GetTrack(project, trackIndex);
				if (track)
					SetTrackSelected(track, true);
			}
		}
		Undo_EndBlock2(project, "Delete track", UNDO_STATE_ALL);
		PreventUIRefresh(-1);
		return;
	}

	// Note: Currently not used and not tested...
	if (std::strcmp(cmd, "clearAutomation") == 0)
	{
		this->DeleteAllAutomationEnvelopes(project, track);
		return;
	}

	if (std::strcmp(cmd, "recordClip") == 0)
	{
		RecordMidiClip(project, track);
		return;
	}

	if (std::strcmp(cmd, "clip") == 0)
	{
		if (path.size() < 4)
			return;
		const int clipIndex = atoi(SafeGet(path, 2));
		MediaItem* item = GetTrackMediaItem(track, clipIndex);
		if (item == nullptr)
			return;

		const char* subcmd = SafeGet(path, 3);

		if (std::strcmp(subcmd, "select") == 0)
		{
			Main_OnCommandEx(UNSELECT_ALL_ITEMS, 0, project);
			SetMediaItemSelected(item, true);
			UpdateTimeline();
			TrackFX_Show(track, this->model.deviceSelected, this->model.deviceExpandedType ? 1 : 3);
			return;
		}

		if (std::strcmp(subcmd, "launch") == 0)
		{
			const double position = GetMediaItemInfo_Value(item, "D_POSITION");
			SetEditCurPos2(project, position, true, true);
			if ((GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
			return;
		}

		if (std::strcmp(subcmd, "record") == 0)
		{
			const double position = GetMediaItemInfo_Value(item, "D_POSITION");
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

		if (std::strcmp(subcmd, "duplicate") == 0)
		{
			Main_OnCommandEx(UNSELECT_ALL_ITEMS, 0, project);
			SetMediaItemSelected(item, true);
			Main_OnCommandEx(DUPLICATE_ITEMS, 0, project);
			UpdateTimeline();
			Undo_OnStateChangeEx("Duplicate item", UNDO_STATE_ITEMS, -1);
			return;
		}

		return;
	}
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::deque<std::string>& path, int value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(SafeGet(path, 0)));
	if (trackIndex < 0)
		return;
	MediaTrack* track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char* cmd = SafeGet(path, 1);

	if (std::strcmp(cmd, "select") == 0)
	{
		SetOnlyTrackSelected(track);
		ScrollTrackIntoView(track);
		const int deviceCount = TrackFX_GetCount(track);
		if (this->model.deviceSelected >= deviceCount)
			this->model.deviceSelected = 0;
		return;
	}

	if (std::strcmp(cmd, "isGroupExpanded") == 0)
	{
		int folderCompact = value > 0 ? 0 : 2;
		GetSetMediaTrackInfo(track, "I_FOLDERCOMPACT", static_cast<void*> (&folderCompact));
		return;
	}

	if (std::strcmp(cmd, "createClip") == 0)
	{
		CreateMidiClip(project, track, value);
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
		CSurf_OnRecArmChange(track, value);
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

	if (std::strcmp(cmd, "pin") == 0)
	{
		this->model.pinnedTrackIndex = value;
		return;
	}

	if (std::strcmp(cmd, "overdub") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_RECMODE", value > 0 ? 7 : 8);
		return;
	}

	if (TrackProcessor::ProcessAutomation(track, cmd, value))
		return;

	Process(path, static_cast<double>(value));
}


bool TrackProcessor::ProcessAutomation(MediaTrack* track, const char* cmd, const int& value) const noexcept
{
	if (value <= 0)
		return false;

	int mode = -1;

	if (std::strcmp(cmd, "autotrim_read") == 0)
		mode = 0;
	else if (std::strcmp(cmd, "autoread") == 0)
		mode = 1;
	else if (std::strcmp(cmd, "autotouch") == 0)
		mode = 2;
	else if (std::strcmp(cmd, "autowrite") == 0)
		mode = 3;
	else if (std::strcmp(cmd, "autolatch") == 0)
		mode = 4;
	else if (std::strcmp(cmd, "autolatch_preview") == 0)
		mode = 5;

	if (mode < 0)
		return false;

	SetTrackAutomationMode(track, mode);
	CSurf_SetAutoMode(-1, nullptr);
	return true;
}


/** {@inheritDoc} */
void TrackProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	const int trackIndex = GetTrackIndex(project, atoi(SafeGet(path, 0)));
	if (trackIndex < 0)
		return;
	MediaTrack* track = GetTrack(project, trackIndex);
	if (!track)
		return;

	std::unique_ptr<Track>& trackData = this->model.GetTrack(trackIndex);
	const char* cmd = SafeGet(path, 1);

	if (std::strcmp(cmd, "volume") == 0)
	{
		if (path.size() == 2)
		{
			trackData->volume = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			CSurf_SetSurfaceVolume(track, CSurf_OnVolumeChange(track, trackData->volume, false), nullptr);
			return;
		}

		const char* touchCmd = SafeGet(path, 2);
		if (std::strcmp(touchCmd, "touch") == 0)
		{
			trackData->isVolumeTouch = value > 0;
		}
		return;
	}

	if (std::strcmp(cmd, "pan") == 0)
	{
		if (path.size() == 2)
		{
			trackData->pan = value * 2 - 1;
			CSurf_SetSurfacePan(track, CSurf_OnPanChange(track, trackData->pan, false), nullptr);
			return;
		}

		const char* touchCmd = SafeGet(path, 2);
		if (std::strcmp(touchCmd, "touch") == 0)
		{
			trackData->isPanTouch = value > 0;
		}
		return;
	}

	if (std::strcmp(cmd, "send") == 0)
	{
		const int sendIndex = atoi(SafeGet(path, 2));
		const char* subcmd = SafeGet(path, 3);
		if (std::strcmp(subcmd, "volume") == 0)
		{
			DISABLE_WARNING_DANGLING_POINTER
			const std::unique_ptr<Send>& send = trackData->GetSend(sendIndex);
			send->volume = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			CSurf_OnSendVolumeChange(track, sendIndex, send->volume, false);
		}
		return;
	}

	if (std::strcmp(cmd, "inQuantResolution") == 0)
	{
		if (value < 0 || value > 1)
			return;
		char chunk[Track::CHUNK_LENGTH];
		DISABLE_WARNING_ARRAY_POINTER_DECAY
			if (!GetTrackStateChunk(track, chunk, Track::CHUNK_LENGTH, false))
				return;

		if (value == 0)
		{
			Main_OnCommandEx(DISABLE_MIDI_INPUT_QUANTIZE, 0, project);
		}
		else
		{
			Main_OnCommandEx(ENABLE_MIDI_INPUT_QUANTIZE, 0, project);
			if (value == 1)
				Main_OnCommandEx(SET_MIDI_INPUT_QUANTIZE_1_4, 0, project);
			else if (value == 0.5)
				Main_OnCommandEx(SET_MIDI_INPUT_QUANTIZE_1_8, 0, project);
			else if (value == 0.25)
				Main_OnCommandEx(SET_MIDI_INPUT_QUANTIZE_1_16, 0, project);
			else if (value == 0.125)
				Main_OnCommandEx(SET_MIDI_INPUT_QUANTIZE_1_32, 0, project);
		}
		return;
	}

	// Parse user parameter value
	if (std::strcmp(cmd, "user") == 0)
	{
		if (path.empty())
			return;
		const char* part = SafeGet(path, 2);
		if (std::strcmp(part, "param") == 0)
		{
			const int userParamNo = atoi(SafeGet(path, 3));
			int fxindexOut;
			int parmidxOut;
			if (!GetTCPFXParm(project, track, userParamNo, &fxindexOut, &parmidxOut))
				return;
			if (std::strcmp(SafeGet(path, 4), "value") == 0)
				TrackFX_SetParamNormalized(track, fxindexOut, parmidxOut, value);
		}
		return;
	}
}


void TrackProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	const char* param = SafeGet(path, 0);
	const int dawIndex = atoi(param);
	const int trackIndex = GetTrackIndex(project, dawIndex);
	if (trackIndex < 0)
		return;
	MediaTrack* track = GetTrack(project, trackIndex);
	if (!track)
		return;

	const char* cmd = SafeGet(path, 1);
	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfTrack(project, track, value);
		return;
	}

	if (std::strcmp(cmd, "name") == 0)
	{
		try
		{
			std::string val = value;

			Undo_BeginBlock2(project);
			GetSetMediaTrackInfo(track, "P_NAME", &val[0]);
			Undo_EndBlock2(project, "Set track name", UNDO_STATE_ALL);
		}
		catch (...)
		{
			// Can't do anything about it...
			return;
		}

		return;
	}
}


void TrackProcessor::Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();

	const char* cmd = SafeGet(path, 0);

	if (std::strcmp(cmd, "addTrack") == 0)
	{
		PreventUIRefresh(1);
		Undo_BeginBlock2(project);

		// Add track
		MediaTrack* track = GetSelectedTrack(project, 0);
		const int trackID = std::max(0, std::min(GetNumTracks(), track == nullptr ? 0 : CSurf_TrackToID(track, false) + 1));
		InsertTrackAtIndex(trackID, true);
		track = GetTrack(project, trackID);
		if (track != nullptr)
		{
			// Select the track and make it visible
			SetOnlyTrackSelected(track);
			ScrollTrackIntoView(track);

			// First parameter is the type
			const char* type = values.at(0).c_str();
			const bool isInstrument = std::strcmp(type, "INSTRUMENT") == 0;

			// Second parameter is the name, if empty generate one
			std::string name = values.at(1);
			if (name.length() == 0)
			{
				if (isInstrument)
					name = "Instrument ";
				else if (std::strcmp(type, "AUDIO") == 0)
					name = "Audio ";
				else
					name = "Effect ";
				name = MakeString() << name << trackID + 1;
			}

			// Give it a name
			GetSetMediaTrackInfo_String(track, "P_NAME", const_cast<char*>(name.c_str()), true);

			if (isInstrument)
			{
				// Set recording input to all MIDI inputs
				SetMediaTrackInfo_Value(track, "I_RECINPUT", 4096 | (63 << 5));
				// Activate MIDI overdub
				SetMediaTrackInfo_Value(track, "I_RECMODE", 7);
			}

			// Set the color if present
			if (strlen(values.at(2).c_str()) > 0)
			{
				int red{ 0 };
				int green{ 0 };
				int blue{ 0 };
				try
				{
					std::cmatch result;
					if (!std::regex_search(values.at(2).c_str(), result, colorPattern))
						return;
					red = std::atoi(result.str(1).c_str());
					green = std::atoi(result.str(2).c_str());
					blue = std::atoi(result.str(3).c_str());
				}
				catch (...)
				{
					return;
				}

				SetTrackColor(track, ColorToNative(red, green, blue));
			}

			// Add devices, if any
			for (size_t i = 3; i < values.size(); i++)
			{
				const char* deviceName = values.at(i).c_str();
				const int position = TrackFX_AddByName(track, deviceName, false, -1);
				if (position >= 0)
					TrackFX_CopyToTrack(track, position, track, 3 - static_cast<int> (i), true);
			}
		}

		Undo_EndBlock2(project, "Add track", UNDO_STATE_ALL);
		PreventUIRefresh(-1);
		return;
	}
}


void TrackProcessor::CreateMidiClip(ReaProject* project, MediaTrack* track, int beats) noexcept
{
	Undo_BeginBlock2(project);

	// Stop playback to update the play cursor position
	Main_OnCommandEx(TRANSPORT_STOP, 0, project);

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

	MediaItem* item = CreateNewMIDIItemInProj(track, cursorPos, end, nullptr);
	if (item != nullptr)
	{
		// Set the loop to the item
		GetSet_LoopTimeRange(true, true, &cursorPos, &end, false);
		SetMediaItemSelected(item, true);

		Main_OnCommandEx(TRANSPORT_PLAY, 0, project);
	}

	Undo_EndBlock2(project, "Create Midi Clip and Record", UNDO_STATE_ALL);
}


void TrackProcessor::RecordMidiClip(ReaProject* project, MediaTrack* track) noexcept
{
	Undo_BeginBlock2(project);

	// Stop playback to update the play cursor position
	Main_OnCommandEx(TRANSPORT_STOP, 0, project);

	// Disable recording on all tracks
	for (int idx = 0; idx < CountTracks(project); idx++)
		SetMediaTrackInfo_Value(GetTrack(project, idx), "I_RECARM", 0);
	// Set recording mode to midi overdub
	SetMediaTrackInfo_Value(track, "I_RECMODE", 7);
	// Enable Recording on current track
	SetMediaTrackInfo_Value(track, "I_RECARM", 1);

	Main_OnCommandEx(TRANSPORT_RECORD, 0, project);

	Undo_EndBlock2(project, "Record Midi Clip", UNDO_STATE_ALL);
}


void TrackProcessor::SetColorOfTrack(ReaProject* project, MediaTrack* track, const std::string& value) noexcept
{
	if (track == nullptr)
		return;

	int red{ 0 };
	int green{ 0 };
	int blue{ 0 };
	try
	{
		std::cmatch result;
		if (!std::regex_search(value.c_str(), result, colorPattern))
			return;
		red = std::atoi(result.str(1).c_str());
		green = std::atoi(result.str(2).c_str());
		blue = std::atoi(result.str(3).c_str());
	}
	catch (...)
	{
		return;
	}

	Undo_BeginBlock2(project);
	SetTrackColor(track, ColorToNative(red, green, blue));
	Undo_EndBlock2(project, "Set track color", UNDO_STATE_ALL);
}


void TrackProcessor::SetIsActivated(ReaProject* project, bool enable) noexcept
{
	Undo_BeginBlock2(project);
	if (enable)
	{
		Main_OnCommandEx(UNLOCK_TRACK_CONTROLS, 0, project);
		Main_OnCommandEx(SET_ALL_FX_ONLINE, 0, project);
		Main_OnCommandEx(UNMUTE_TRACKS, 0, project);
		Undo_EndBlock2(project, "Enable track", UNDO_STATE_ALL);
	}
	else
	{
		Main_OnCommandEx(MUTE_TRACKS, 0, project);
		Main_OnCommandEx(SET_ALL_FX_OFFLINE, 0, project);
		Main_OnCommandEx(LOCK_TRACK_CONTROLS, 0, project);
		Undo_EndBlock2(project, "Disable track", UNDO_STATE_ALL);
	}
}


int TrackProcessor::GetTrackIndex(ReaProject* project, int dawTrackIndex) const noexcept
{
	const int count = CountTracks(project);
	if (dawTrackIndex < 0 || dawTrackIndex >= count)
		return -1;

	int trackIndex{ 0 };
	int trackState{};
	for (int index = 0; index < count; index++)
	{
		MediaTrack* mediaTrack = GetTrack(project, index);
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


void TrackProcessor::DeleteAllAutomationEnvelopes(ReaProject* project, MediaTrack* track) noexcept
{
	PreventUIRefresh(1);
	Undo_BeginBlock2(project);

	const double end = GetProjectLength(project);

	const int count = CountTrackEnvelopes(track);
	for (int i = 0; i < count; i++)
	{
		TrackEnvelope* envelope = GetTrackEnvelope(track, i);
		DeleteEnvelopePointRange(envelope, 0, end);
	}

	Undo_EndBlock2(project, "Delete all automation envelopes of track.", UNDO_STATE_TRACKCFG);
	PreventUIRefresh(-1);
}


void TrackProcessor::ScrollTrackIntoView(MediaTrack* leftmosttrack) noexcept
{
	SetMixerScroll(leftmosttrack);
	ReaProject* project = ReaperUtils::GetProject();
	Main_OnCommandEx(VERTICAL_SCROLL_TRACK_INTO_VIEW, 0, project);
}
