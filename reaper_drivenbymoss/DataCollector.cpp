// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <fstream>

#include "WrapperGSL.h"
#include "WrapperReaperFunctions.h"
#include "DataCollector.h"
#include "Collectors.h"
#include "NoteRepeatProcessor.h"
#include "ReaperUtils.h"
#include "ReaDebug.h"


/**
 * Constructor.
 *
 * @param aModel The model
 */
DataCollector::DataCollector(Model& aModel) noexcept :
	model(aModel),
	deviceSiblings(aModel.DEVICE_BANK_SIZE, ""),
	instrumentParameter("/primary/param/", 0),
	devicePresetsStr(128, "")
{
	this->trackStateChunk = std::make_unique<char[]>(BUFFER_SIZE);
}


/**
 * Destructor.
 */
DataCollector::~DataCollector()
{
	this->trackStateChunk.reset();
}


/**
 * Collect all (changed) data.
 *
 * @param dump If true all data is collected not only the changed one since the last call
 * @return The formatted data in OSC style separated by line separators
 */
std::string DataCollector::CollectData(const bool& dump)
{
	std::ostringstream ss;
	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack(project, 0);

	this->slowCounter = (this->slowCounter + 1) % SLOW_UPDATE;

	if (IsActive("transport"))
		CollectTransportData(ss, project, dump);
	if (IsActive("project"))
		CollectProjectData(ss, project, dump);
	if (IsActive("track"))
		CollectTrackData(ss, project, dump);
	if (IsActive("device"))
		CollectDeviceData(ss, project, track, dump);
	if (IsActive("mastertrack"))
		CollectMasterTrackData(ss, project, dump);
	if (IsActive("browser"))
		CollectBrowserData(ss, track, dump);
	if (IsActive("marker"))
		CollectMarkerData(ss, project, dump);
	if (IsActive("clip"))
		CollectClipData(ss, project, dump);
	if (IsActive("session"))
		CollectSessionData(ss, project, dump);
	if (IsActive("noterepeat"))
		CollectNoteRepeatData(ss, project, dump);

	return ss.str();
}


/**
 * Collect the (changed) transport data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectTransportData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	// Transport states
	const int playState = GetPlayStateEx(project);
	this->play = Collectors::CollectIntValue(ss, "/play", this->play, (playState & 1) > 0, dump);
	this->record = Collectors::CollectIntValue(ss, "/record", this->record, (playState & 4) > 0, dump);
	this->repeat = Collectors::CollectIntValue(ss, "/repeat", this->repeat, GetSetRepeat(-1), dump);
	this->metronome = Collectors::CollectIntValue(ss, "/click", this->metronome, GetToggleCommandState(40364), dump);
	this->prerollClick = Collectors::CollectIntValue(ss, "/prerollClick", this->prerollClick, GetToggleCommandState(41819), dump);

	// The tempo
	this->tempo = Collectors::CollectDoubleValue(ss, "/tempo", this->tempo, Master_GetTempo(), dump);

	// Get the time signature at the current play position, if playback is active or never was read
	const double cursorPos = ReaperUtils::GetCursorPosition(project);
	int timesig;
	int denomOut;
	double startBPM;
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->globalTimesig = Collectors::CollectIntValue(ss, "/numerator", this->globalTimesig, timesig, dump);
	this->globalDenomOut = Collectors::CollectIntValue(ss, "/denominator", this->globalDenomOut, denomOut, dump);

	// Result is in seconds
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->playPosition = Collectors::CollectDoubleValue(ss, "/time", this->playPosition, cursorPos, dump);

	// Add project offset, if configured in project settings
	const double timeOffset = GetProjectTimeOffset(project, false);
	char timeStr[20];
	format_timestr(timeOffset + cursorPos, timeStr, 20);
	this->strPlayPosition = Collectors::CollectStringValue(ss, "/time/str", this->strPlayPosition, timeStr, dump);

	// 2 = measures.beats
	format_timestr_pos(cursorPos, timeStr, 20, 2);
	this->strBeatPosition = Collectors::CollectStringValue(ss, "/beat", this->strBeatPosition, timeStr, dump);
}


/**
 * Collect the (changed) project data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectProjectData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	if (this->slowCounter == 0)
	{
		char newProjectName[20];
		GetProjectName(project, newProjectName, 20);
		this->projectName = Collectors::CollectStringValue(ss, "/project/name", projectName, newProjectName, dump);
		this->projectEngine = Collectors::CollectIntValue(ss, "/project/engine", projectEngine, Audio_IsRunning(), dump);
	}
}


/**
 * Collect the (changed) device data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param track The currently selected track
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectDeviceData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, const bool& dump)
{
	const int deviceIndex = this->model.deviceBankOffset + this->model.deviceSelected;
	int bankDeviceIndex = 1;
	this->model.deviceCount = Collectors::CollectIntValue(ss, "/device/count", this->model.deviceCount, TrackFX_GetCount(track), dump);
	this->deviceExists = Collectors::CollectIntValue(ss, "/device/exists", this->deviceExists, deviceIndex < this->model.deviceCount ? 1 : 0, dump);
	this->devicePosition = Collectors::CollectIntValue(ss, "/device/position", this->devicePosition, deviceIndex, dump);
	this->deviceWindow = Collectors::CollectIntValue(ss, "/device/window", this->deviceWindow, TrackFX_GetOpen(track, deviceIndex), dump);
	this->deviceExpanded = Collectors::CollectIntValue(ss, "/device/expand", this->deviceExpanded, this->model.deviceExpandedType == 1, dump);

	constexpr int LENGTH = 30;
	char name[LENGTH];

	if (this->slowCounter == 0)
	{
		bool result = TrackFX_GetFXName(track, deviceIndex, name, LENGTH);
		this->deviceName = Collectors::CollectStringValue(ss, "/device/name", this->deviceName, result ? name : "", dump);
		this->deviceBypass = Collectors::CollectIntValue(ss, "/device/bypass", this->deviceBypass, TrackFX_GetEnabled(track, deviceIndex) ? 0 : 1, dump);

		for (int index = 0; index < this->model.DEVICE_BANK_SIZE; index++)
		{
			std::ostringstream das;
			das << "/device/sibling/" << bankDeviceIndex << "/name";
			const std::string deviceAddress = das.str();
			result = TrackFX_GetFXName(track, this->model.deviceBankOffset + index, name, LENGTH);
			Collectors::CollectStringArrayValue(ss, das.str().c_str(), index, deviceSiblings, result ? name : "", dump);
			bankDeviceIndex++;
		}
	}

	const int paramCount = TrackFX_GetNumParams(track, deviceIndex);
	this->model.deviceParamCount = Collectors::CollectIntValue(ss, "/device/param/count", this->model.deviceParamCount, paramCount, dump);
	for (int index = 0; index < paramCount; index++)
	{
		std::shared_ptr<Parameter> parameter = this->model.GetParameter(index);
		parameter->CollectData(ss, track, deviceIndex, dump);
	}

	// 
	// First instrument (primary) data
	// 

	if (this->slowCounter == 0)
	{
		const int instrumentIndex = TrackFX_GetInstrument(track);
		this->instrumentExists = Collectors::CollectIntValue(ss, "/primary/exists", this->instrumentExists, instrumentIndex >= 0, dump);
		this->instrumentPosition = Collectors::CollectIntValue(ss, "/primary/position", this->instrumentPosition, instrumentIndex, dump);
		const bool result = TrackFX_GetFXName(track, instrumentIndex, name, LENGTH);
		this->instrumentName = Collectors::CollectStringValue(ss, "/primary/name", this->instrumentName, result ? name : "", dump);

		// Currently, we only need 1 parameter for the Kontrol OSC ID
		Collectors::CollectIntValue(ss, "/primary/param/count", 1, 1, dump);
		this->instrumentParameter.CollectData(ss, track, instrumentIndex, dump);
	}


	// Track FX Parameter (as user parameters)
	const int userParamCount = CountTCPFXParms(project, track);
	this->model.userParamCount = Collectors::CollectIntValue(ss, "/user/param/count", this->model.userParamCount, userParamCount, dump);
	int fxindexOut;
	int parmidxOut;
	for (int index = 0; index < userParamCount; index++)
	{
		if (GetTCPFXParm(project, track, index, &fxindexOut, &parmidxOut))
		{
			std::shared_ptr<Parameter> parameter = this->model.GetUserParameter(index);
			parameter->CollectData(ss, track, fxindexOut, parmidxOut, dump);
		}
	}
}


/**
 * Collect the (changed) track data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectTrackData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	const int count = CountTracks(project);
	int trackIndex{ 0 };
	int trackState{};
	std::string playingNotes{ "" };

	const bool isActive = IsActive("playingnotes");

	for (int index = 0; index < count; index++)
	{
		MediaTrack* mediaTrack = GetTrack(project, index);
		if (mediaTrack == nullptr)
			continue;

		// Ignore track if hidden
		GetTrackState(mediaTrack, &trackState);
		if ((trackState & 1024) > 0)
			continue;
		std::shared_ptr <Track> track = this->model.GetTrack(trackIndex);
		track->CollectData(ss, project, mediaTrack, trackIndex, this->slowCounter == 0, dump);

		// Only collect note information, if enabled, track is active and playback is on
		if (isActive && this->play > 0 && track->isSelected > 0)
		{
			std::ostringstream das;
			das << "/track/" << trackIndex << "/playingnotes";
			playingNotes = this->CollectPlayingNotes(project, mediaTrack);
			this->playingNotesStr = Collectors::CollectStringValue(ss, das.str().c_str(), this->playingNotesStr, playingNotes.c_str(), dump);
		}

		trackIndex++;
	}
	this->model.trackCount = Collectors::CollectIntValue(ss, "/track/count", this->model.trackCount, trackIndex, dump);
}

std::string DataCollector::CollectPlayingNotes(ReaProject* project, MediaTrack* track)
{
	std::string notesStrNew{ " " };

	MediaItem_Take* take = this->GetMidiTakeAtPlayPosition(project, track);
	if (take == nullptr)
		return notesStrNew;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return notesStrNew;

	int channel{ 0 }, pitch{ 0 }, velocity{ 0 };
	double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };

	std::ostringstream notes;

	for (int i = 0; i < noteCount; ++i)
	{
		MIDI_GetNote(take, i, nullptr, nullptr, &startppqpos, &endppqpos, &channel, &pitch, &velocity);

		musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
		musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);

		if (this->playPosition >= musicalStart && this->playPosition <= musicalEnd)
			notes << musicalStart << ":" << musicalEnd << ":" << channel << ":" << pitch << ":" << velocity << ";";
	}
	return notes.str().c_str();
}

/**
 * Collect the (changed) master track data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectMasterTrackData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	MediaTrack* master = GetMasterTrack(project);

	int trackState;
	GetTrackState(master, &trackState);

	this->masterSelected = Collectors::CollectIntValue(ss, "/master/select", this->masterSelected, (trackState & 2) > 0, dump);
	this->masterMute = Collectors::CollectIntValue(ss, "/master/mute", this->masterMute, (trackState & 8) > 0 ? 1 : 0, dump);
	this->masterSolo = Collectors::CollectIntValue(ss, "/master/solo", this->masterSolo, (trackState & 16) > 0 ? 1 : 0, dump);

	// Master track volume and pan
	const double volDB = ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(master, "D_VOL"));
	this->model.masterVolume = Collectors::CollectDoubleValue(ss, "/master/volume", this->model.masterVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->masterVolumeStr = Collectors::CollectStringValue(ss, "/master/volume/str", this->masterVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);

	const double panVal = GetMediaTrackInfo_Value(master, "D_PAN");
	this->model.masterPan = Collectors::CollectDoubleValue(ss, "/master/pan", this->model.masterPan, (panVal + 1) / 2, dump);
	this->masterPanStr = Collectors::CollectStringValue(ss, "/master/pan/str", this->masterPanStr, Collectors::FormatPan(panVal).c_str(), dump);

	this->masterVULeft = Collectors::CollectDoubleValue(ss, "/master/vuleft", this->masterVULeft, DB2SLIDER(ReaperUtils::ValueToDB(Track_GetPeakInfo(master, 0))) / 1000.0, dump);
	this->masterVURight = Collectors::CollectDoubleValue(ss, "/master/vuright", this->masterVURight, DB2SLIDER(ReaperUtils::ValueToDB(Track_GetPeakInfo(master, 1))) / 1000.0, dump);

	if (this->slowCounter == 0)
	{
		// Track color
		int red = -1, green = -1, blue = -1;
		// Note: GetTrackColor is not working for the master track
		const int nativeColor = gsl::narrow_cast<int> (GetMediaTrackInfo_Value(master, "I_CUSTOMCOLOR"));
		if (nativeColor != 0)
			ColorFromNative(nativeColor & 0xFEFFFFFF, &red, &green, &blue);
		this->masterColor = Collectors::CollectStringValue(ss, "/master/color", this->masterColor, Collectors::FormatColor(red, green, blue).c_str(), dump);

		// Automation mode
		const double automode = GetMediaTrackInfo_Value(master, "I_AUTOMODE");
		this->masterAutoMode = Collectors::CollectIntValue(ss, "/master/automode", this->masterAutoMode, static_cast<int>(automode), dump);
	}
}


/**
 * Collect the (changed) data of the first selected clip.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectClipData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	// Get the selected media item if any and calculate the items start and end
	double musicalStart{ -1 };
	double musicalEnd{ -1 };
	double musicalPlayPosition{ -1 };
	double bpm{};
	const int count = CountSelectedMediaItems(project);
	int red = 0, green = 0, blue = 0;
	MediaItem* item = nullptr;
	int loopIsEnabled = 0;
	this->clipExists = Collectors::CollectIntValue(ss, "/clip/exists", this->clipExists, count > 0 ? 1 : 0, dump);
	if (count > 0)
	{
		item = GetSelectedMediaItem(project, 0);
		const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		const double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		TimeMap_GetTimeSigAtTime(project, itemStart, nullptr, nullptr, &bpm);
		musicalStart = bpm * itemStart / 60;
		TimeMap_GetTimeSigAtTime(project, itemEnd, nullptr, nullptr, &bpm);
		musicalEnd = bpm * itemEnd / 60;

		if (itemStart <= this->playPosition && this->playPosition <= itemEnd)
		{
			TimeMap_GetTimeSigAtTime(project, this->playPosition, nullptr, nullptr, &bpm);
			musicalPlayPosition = bpm * this->playPosition / 60;
		}

		const int clipColor = (int)GetDisplayedMediaItemColor(item);
		ColorFromNative(clipColor & 0xFEFFFFFF, &red, &green, &blue);

		loopIsEnabled = GetMediaItemInfo_Value(item, "B_LOOPSRC") > 0 ? 1 : 0;
	}
	std::string newNotes = this->CollectClipNotes(project, item);
	this->notesStr = Collectors::CollectStringValue(ss, "/clip/notes", this->notesStr, newNotes.c_str(), dump);

	this->clipMusicalStart = Collectors::CollectDoubleValue(ss, "/clip/start", this->clipMusicalStart, musicalStart, dump);
	this->clipMusicalEnd = Collectors::CollectDoubleValue(ss, "/clip/end", this->clipMusicalEnd, musicalEnd, dump);
	this->clipMusicalPlayPosition = Collectors::CollectDoubleValue(ss, "/clip/playposition", this->clipMusicalPlayPosition, musicalPlayPosition, dump);

	this->clipLoopIsEnabled = Collectors::CollectIntValue(ss, "/clip/loop", this->clipLoopIsEnabled, loopIsEnabled, dump);

	this->clipColor = Collectors::CollectStringValue(ss, "/clip/color", this->clipColor, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


/**
 * Collect the notes data of the first selected clip if changed.
 *
 * @param project The current Reaper project
 * @param item The media item
 * @return The formatted collected notes
 */
std::string DataCollector::CollectClipNotes(ReaProject* project, MediaItem* item)
{
	std::string notesStrNew{ " " };

	if (item == nullptr)
	{
		this->noteHash = "";
		return notesStrNew;
	}
	MediaItem_Take* take = GetActiveTake(item);
	if (take == nullptr || !TakeIsMIDI(take))
	{
		this->noteHash = "";
		return notesStrNew;
	}

	// Have notes changed since last call?
	char nhash[16];
	if (!MIDI_GetHash(take, true, nhash, 16) || this->noteHash.compare(nhash) == 0)
		return this->notesStr;
	this->noteHash = nhash;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return notesStrNew;

	int channel{ 0 }, pitch{ 0 }, velocity{ 0 };
	double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };
	double bpm{};
	std::ostringstream notes;
	const double pos = GetMediaItemInfo_Value(item, "D_POSITION");

	for (int i = 0; i < noteCount; ++i)
	{
		MIDI_GetNote(take, i, nullptr, nullptr, &startppqpos, &endppqpos, &channel, &pitch, &velocity);

		musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
		musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);
		musicalStart -= pos;
		musicalEnd -= pos;

		TimeMap_GetTimeSigAtTime(project, startppqpos, nullptr, nullptr, &bpm);
		musicalStart = bpm * musicalStart / 60.0;
		TimeMap_GetTimeSigAtTime(project, endppqpos, nullptr, nullptr, &bpm);
		musicalEnd = bpm * musicalEnd / 60.0;
		notes << musicalStart << ":" << musicalEnd << ":" << channel << ":" << pitch << ":" << velocity << ";";
	}
	return notes.str().c_str();
}


/**
 * Collect the (changed) browser data.
 *
 * @param ss The stream where to append the formatted data
 * @param track The currently selected track
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectBrowserData(std::ostringstream& ss, MediaTrack* track, const bool& dump)
{
	if (this->slowCounter != 0)
		return;

	const int sel = this->model.deviceBankOffset + this->model.deviceSelected;

	LoadDevicePresetFile(ss, track, sel, dump);

	constexpr int LENGTH = 20;
	char presetname[LENGTH];
	TrackFX_GetPreset(track, sel, presetname, LENGTH);
	this->devicePresetName = Collectors::CollectStringValue(ss, "/browser/selected/name", this->devicePresetName, presetname, dump);
	int numberOfPresets;
	const int selectedIndex = TrackFX_GetPresetIndex(track, this->model.deviceBankOffset + this->model.deviceSelected, &numberOfPresets);
	this->devicePresetIndex = Collectors::CollectIntValue(ss, "/browser/selected/index", this->devicePresetIndex, selectedIndex, dump);
}


/**
 * Collect the (changed) marker data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectMarkerData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	if (this->slowCounter == 0)
	{
		const std::vector<int> markers = Marker::GetMarkers(project);
		const int count = (int)markers.size();
		this->model.markerCount = Collectors::CollectIntValue(ss, "/marker/count", this->model.markerCount, count, dump);
		for (int index = 0; index < count; index++)
		{
			std::shared_ptr <Marker> marker = this->model.GetMarker(markers.at(index));
			marker->CollectData(ss, project, "marker", index, markers.at(index), dump);
		}
	}
}


/**
 * Collect the (changed) session data (clips are Media items, scenes are Region markers).
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectSessionData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	// Only collect clip data if document has changed
	const int state = GetProjectStateChangeCount(project);
	if (this->projectState == state)
		return;
	this->projectState = state;

	std::ostringstream clipstr;

	int count = CountTracks(project);
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

		int red = 0, green = 0, blue = 0;
		char buf[2048];

		const int itemCount = CountTrackMediaItems(mediaTrack);

		clipstr << trackIndex << ";" << itemCount << ";";

		for (int i = 0; i < itemCount; i++)
		{
			MediaItem* item = GetTrackMediaItem(mediaTrack, i);
			if (item == nullptr)
				continue;
			MediaItem_Take* take = GetActiveTake(item);
			if (take == nullptr)
				continue;

			// Format track index, clip index, name, selected state and color in one string

			if (GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false))
			{
				std::string s{ buf };
				std::replace(s.begin(), s.end(), ';', ' ');
				clipstr << s;
			}
			else
			{
				ReaDebug() << "ERROR: Could not read name from clip " << i << " on track " << (trackIndex + 1);
				clipstr << "Unknown";
			}

			const bool isSelected = IsMediaItemSelected(item);
			clipstr << ";" << isSelected;

			const int clipColor = (int)GetDisplayedMediaItemColor(item);
			ColorFromNative(clipColor & 0xFEFFFFFF, &red, &green, &blue);
			clipstr << ";" << Collectors::FormatColor(red, green, blue).c_str() << ";";
		}

		trackIndex++;
	}
	this->formattedClips = Collectors::CollectStringValue(ss, "/clip/all", this->formattedClips, clipstr.str().c_str(), dump);

	// Collect "scenes", use Region markers
	const std::vector<int> regions = Marker::GetRegions(project);
	count = (int)regions.size();
	this->model.sceneCount = Collectors::CollectIntValue(ss, "/scene/count", this->model.sceneCount, count, dump);
	for (int index = 0; index < count; index++)
	{
		std::shared_ptr <Marker> scene = this->model.GetRegion(regions.at(index));
		scene->CollectData(ss, project, "scene", index, regions.at(index), dump);
	}
}


/**
 * Collect the NoteRepeat data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectNoteRepeatData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	MediaTrack* const track = GetSelectedTrack(project, 0);

	// Midi note repeat plugin is on track?
	const int position = track ? TrackFX_AddByName(track, NoteRepeatProcessor::MIDI_ARP_PLUGIN, 1, 0) : -1;
	const int inputPosition = 0x1000000 + position;

	const int repeatActive = position > -1 && TrackFX_GetEnabled(track, inputPosition) ? 1 : 0;
	this->repeatActive = Collectors::CollectIntValue(ss, "/noterepeat/active", this->repeatActive, repeatActive ? 1 : 0, dump);

	double minVal{}, maxVal{};
	const double repeatRate = position > -1 ? TrackFX_GetParam(track, inputPosition, NoteRepeatProcessor::MIDI_ARP_PARAM_RATE, &minVal, &maxVal) : 1.0;
	this->repeatRate = Collectors::CollectDoubleValue(ss, "/noterepeat/period", this->repeatRate, repeatRate, dump);

	const double repeatNoteLength = position > -1 ? TrackFX_GetParam(track, inputPosition, NoteRepeatProcessor::MIDI_ARP_PARAM_NOTE_LENGTH, &minVal, &maxVal) : 1.0;
	this->repeatNoteLength = Collectors::CollectDoubleValue(ss, "/noterepeat/notelength", this->repeatNoteLength, repeatNoteLength, dump);

	const double repeatMode = position > -1 ? TrackFX_GetParam(track, inputPosition, NoteRepeatProcessor::MIDI_ARP_PARAM_MODE, &minVal, &maxVal) : 0;
	this->repeatMode = Collectors::CollectIntValue(ss, "/noterepeat/mode", this->repeatMode, (int)repeatMode, dump);

	const double repeatVelocity = position > -1 ? TrackFX_GetParam(track, inputPosition, NoteRepeatProcessor::MIDI_ARP_PARAM_VELOCITY, &minVal, &maxVal) : 0;
	this->repeatVelocity = Collectors::CollectIntValue(ss, "/noterepeat/velocity", this->repeatVelocity, repeatVelocity == 0 ? 1 : 0, dump);
}


/**
 * Delay updates for a specific processor. Use to prevent that Reaper sends old
 * values before the latest ones are applied.
 *
 * @param processor The processor to delay
 */
void DataCollector::DelayUpdate(std::string processor)
{
	const std::lock_guard<std::mutex> lock(this->delayMutex);
	this->delayUpdateMap[processor] = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
}


/**
 * Dis-/enable an update processor for performance improvements.
 *
 * @param processor The processor to dis-/enable
 * @param enable True to enable processor updates, false to disable
 */
void DataCollector::EnableUpdate(std::string processor, bool enable)
{
	this->disableUpdateMap[processor] = !enable;
}


/**
 * Tests if the given processor is currently active (it is not disabled and not delayed).
 *
 * @param processor The processor to check
 * @return True if the processor is active and not delayed
 */
bool DataCollector::IsActive(std::string processor)
{
	if (this->disableUpdateMap[processor])
		return false;
	return this->CheckDelay(processor);
}


/**
 * Tests if the given processor is currently delayed and therefore no updated data
 * should be sent.
 *
 * @param processor The processor to check if it is delayed
 * @return True if the processor is not delayed
 */
bool DataCollector::CheckDelay(std::string processor)
{
	this->delayMutex.lock();
	bool result = true;
	std::map<std::string, long long>::iterator it = this->delayUpdateMap.find(processor);
	if (it != this->delayUpdateMap.end())
	{
		const long long oldValue = it->second;
		const long long millis = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		if (millis - oldValue < DELAY)
			result = false;
		else
			this->delayUpdateMap.erase(processor);
	}
	this->delayMutex.unlock();
	return result;
}


/**
 * Find the first MIDI item on the track which is under the cursor position (if any).
 *
 * @param project The current Reaper project
 * @param track The track on which to find the take
 * @return The active take of the media item at the play position
 */
MediaItem_Take* DataCollector::GetMidiTakeAtPlayPosition(ReaProject* project, MediaTrack* track) const noexcept
{
	for (int i = 0; i < CountTrackMediaItems(track); i++)
	{
		MediaItem* item = GetTrackMediaItem(track, i);
		MediaItem_Take* take = GetActiveTake(item);
		if (take == nullptr || !TakeIsMIDI(take))
			continue;

		// Ignore muted items
		if (GetMediaItemInfo_Value(item, "B_MUTE") > 0)
			continue;

		const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		const double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		if (this->playPosition >= itemStart && this->playPosition <= itemEnd)
			return take;
	}
	return nullptr;
}


void DataCollector::LoadDevicePresetFile(std::ostringstream& ss, MediaTrack* track, int fx, const bool& dump)
{
	constexpr int LENGTH = 1024;
	char filename[LENGTH];
	TrackFX_GetUserPresetFilename(track, fx, filename, LENGTH);

	try
	{
		std::ifstream file;
		std::string name;
		int counter = 0;

		file.open(filename);
		while (file.good())
		{
			std::getline(file, name);

			std::cmatch result;
			if (std::regex_search(name.c_str(), result, presetPattern))
			{
				std::string strip = result.str(1);

				std::ostringstream das;
				das << "/browser/result/" << counter + 1 << "/name";
				Collectors::CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, strip.c_str(), dump);

				counter += 1;
			}
		}
		file.close();

		while (counter < 128)
		{
			std::ostringstream das;
			das << "/browser/result/" << counter + 1 << "/name";
			Collectors::CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, "", dump);
			counter += 1;
		}
	}
	catch (const std::ios_base::failure & ex)
	{
		(void)ex;
		// File does not exist
		return;
	}
}
