// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
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
DataCollector::DataCollector(Model& aModel) :
	model(aModel),
	deviceSiblings(aModel.DEVICE_BANK_SIZE, ""),
	deviceSiblingsSelection(aModel.DEVICE_BANK_SIZE, 0),
	deviceSiblingsBypass(aModel.DEVICE_BANK_SIZE, 0),
	deviceSiblingsPosition(aModel.DEVICE_BANK_SIZE, 0)
{
	this->trackStateChunk = std::make_unique<char[]>(BUFFER_SIZE);

	for (int i = 0; i < 8; i++)
		this->eqBandTypes.push_back("-1");
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
std::string DataCollector::CollectData(const bool& dump, ActionProcessor& actionProcessor)
{
	std::ostringstream ss;

	actionProcessor.CollectData(ss);

	ReaProject* project = ReaperUtils::GetProject();

	MediaTrack* track{ nullptr };
	if (this->model.pinnedTrackIndex >= 0 && this->model.pinnedTrackIndex < CountTracks(project))
		track = GetTrack(project, this->model.pinnedTrackIndex);
	else
		track = GetSelectedTrack2(project, 0, true);
	const bool hasTrackChanged = selectedTrack != track;
	selectedTrack = track;

	this->slowCounter = (this->slowCounter + 1) % SLOW_UPDATE;

	if (IsActive("project"))
		CollectProjectData(ss, project, dump);
	if (IsActive("transport"))
		CollectTransportData(ss, project, dump);
	if (IsActive("track"))
		CollectTrackData(ss, project, dump);
	if (IsActive("device"))
		CollectDeviceData(ss, project, track, dump || hasTrackChanged);
	if (IsActive("master"))
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
	if (IsActive("groove"))
		CollectGrooveData(ss, project, dump);

	return ss.str();
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
	if (this->slowCounter == 0 || dump)
	{
		constexpr int BUFFER_LENGTH = 50;
		std::string strBuffer(BUFFER_LENGTH, 0);
		char* strBufferPointer = &*strBuffer.begin();
		GetProjectName(project, strBufferPointer, BUFFER_LENGTH);

		this->projectName = Collectors::CollectStringValue(ss, "/project/name", this->projectName, strBuffer, dump);
		this->projectEngine = Collectors::CollectIntValue(ss, "/project/engine", this->projectEngine, Audio_IsRunning(), dump);
	}

	this->canUndo = Collectors::CollectIntValue(ss, "/project/canUndo", this->canUndo, Undo_CanUndo2(project) == nullptr ? 0 : 1, dump);
	this->canRedo = Collectors::CollectIntValue(ss, "/project/canRedo", this->canRedo, Undo_CanRedo2(project) == nullptr ? 0 : 1, dump);
	this->isDirty = Collectors::CollectIntValue(ss, "/project/isDirty", this->isDirty, IsProjectDirty(project), dump);
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
	// The tempo
	this->tempo = Collectors::CollectDoubleValue(ss, "/tempo", this->tempo, Master_GetTempo(), dump);

	// Metronome settings
	this->metronome = Collectors::CollectIntValue(ss, "/click", this->metronome, GetToggleCommandState(40364), dump);
	this->prerollClick = Collectors::CollectIntValue(ss, "/click/preroll", this->prerollClick, GetToggleCommandState(41819), dump);

	constexpr int STR_LENGTH = 20;
	std::string strBuffer(STR_LENGTH, 0);
	char* strBufferPointer = &*strBuffer.begin();

	double value = 0.5;
	if (get_config_var_string("projmetrov1", strBufferPointer, STR_LENGTH))
	{
		replaceCommaWithDot(strBuffer);
		value = std::atof(strBufferPointer);
	}
	const double volDB = ReaperUtils::ValueToDB(value);
	this->metronomeVolume = Collectors::CollectDoubleValue(ss, "/click/volume", this->metronomeVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->metronomeVolumeStr = Collectors::CollectStringValue(ss, "/click/volumeStr", this->metronomeVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);

	bool result = get_config_var_string("preroll", strBufferPointer, STR_LENGTH);
	if (result)
		replaceCommaWithDot(strBuffer);
	this->preRoll = Collectors::CollectStringValue(ss, "/click/preroll", this->preRoll, result ? strBufferPointer : "", dump);
	result = get_config_var_string("prerollmeas", strBufferPointer, STR_LENGTH);
	if (result)
		replaceCommaWithDot(strBuffer);
	this->preRollMeasures = Collectors::CollectStringValue(ss, "/click/prerollMeasures", this->preRollMeasures, result ? strBufferPointer : "", dump);

	// Get the time signature at the current play position, if playback is active or never was read
	const double cursorPos = ReaperUtils::GetCursorPosition(project);
	int timesig;
	int denomOut;
	double startBPM;
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->globalTimesig = Collectors::CollectIntValue(ss, "/numerator", this->globalTimesig, timesig, dump);
	this->globalDenomOut = Collectors::CollectIntValue(ss, "/denominator", this->globalDenomOut, denomOut, dump);

	std::string timeStr(TIME_LENGTH, 0);
	char* timeStrPointer = &*timeStr.begin();

	// Result is in seconds
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->playPosition = Collectors::CollectDoubleValue(ss, "/time", this->playPosition, cursorPos, dump);
	// Add project offset, if configured in project settings
	const double timeOffset = GetProjectTimeOffset(project, false);
	format_timestr(timeOffset + cursorPos, timeStrPointer, TIME_LENGTH);
	this->strPlayPosition = Collectors::CollectStringValue(ss, "/time/str", this->strPlayPosition, timeStrPointer, dump);
	// 2 = measures.beats
	format_timestr_pos(cursorPos, timeStrPointer, TIME_LENGTH, 2);
	replaceCommaWithDot(timeStr);
	this->strBeatPosition = Collectors::CollectStringValue(ss, "/beat", this->strBeatPosition, timeStrPointer, dump);

	// Loop start and length
	double startOut;
	double endOut;
	GetSet_LoopTimeRange(false, true, &startOut, &endOut, false);

	this->loopStart = Collectors::CollectDoubleValue(ss, "/time/loop/start", this->loopStart, startOut, dump);
	format_timestr(timeOffset + startOut, timeStrPointer, TIME_LENGTH);
	this->strLoopStart = Collectors::CollectStringValue(ss, "/time/loop/start/str", this->strLoopStart, timeStr, dump);
	format_timestr_pos(startOut, timeStrPointer, TIME_LENGTH, 2);
	replaceCommaWithDot(timeStr);
	this->strLoopStartBeat = Collectors::CollectStringValue(ss, "/time/loop/start/beat", this->strLoopStartBeat, timeStr, dump);

	this->loopLength = Collectors::CollectDoubleValue(ss, "/time/loop/length", this->loopLength, endOut, dump);
	const double length = endOut - startOut;
	format_timestr_len(length, timeStrPointer, TIME_LENGTH, startOut, 0);
	replaceCommaWithDot(timeStr);
	this->strLoopLength = Collectors::CollectStringValue(ss, "/time/loop/length/str", this->strLoopLength, timeStr, dump);
	format_timestr_len(length, timeStrPointer, TIME_LENGTH, startOut, 2);
	replaceCommaWithDot(timeStr);
	this->strLoopLengthBeat = Collectors::CollectStringValue(ss, "/time/loop/length/beat", this->strLoopLengthBeat, timeStr, dump);

	// Additional info
	this->followPlayback = Collectors::CollectIntValue(ss, "/followPlayback", this->followPlayback, GetToggleCommandState(40036), dump);

	this->automationMode = Collectors::CollectIntValue(ss, "/automode", this->automationMode, GetGlobalAutomationOverride(), dump);
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
	this->model.deviceCount = Collectors::CollectIntValue(ss, "/device/count", this->model.deviceCount, TrackFX_GetCount(track), dump);
	int deviceIndex = this->model.deviceCount == 0 ? -1 : this->model.GetDeviceSelection();

	if (this->model.deviceCount > 0)
	{
		// Ensure that the device index is in the range of available devices
		if (deviceIndex > 0 && deviceIndex >= this->model.deviceCount && this->model.deviceCount > 0)
			this->model.SetDeviceSelection(0);

		// Check for auto-follow
		int trackIndex, itemIndex, takeIndex, fxIndex;
		if (GetTouchedOrFocusedFX(1, &trackIndex, &itemIndex, &takeIndex, &fxIndex, nullptr))
		{
			// Only accept track FX
			if (trackIndex != -1 && fxIndex != -1 && itemIndex == -1 && takeIndex == -1 && fxIndex < 0x1000000)
			{
				MediaTrack* mediaTrack = GetTrack(project, trackIndex);
				// Ensure that the track is selected. We cannot select it because otherwise the track can never be 
				// de-selected again since there is currently no way to clear or set the focused device state
				if (mediaTrack == track && (deviceIndex != fxIndex || mediaTrack != track))
					this->model.SetDeviceSelection(fxIndex);
			}
		}

		deviceIndex = this->model.GetDeviceSelection();
	}

	this->deviceExists = Collectors::CollectIntValue(ss, "/device/exists", this->deviceExists, deviceIndex >= 0 ? 1 : 0, dump);
	this->devicePosition = Collectors::CollectIntValue(ss, "/device/position", this->devicePosition, deviceIndex, dump);
	this->deviceWindow = Collectors::CollectIntValue(ss, "/device/window", this->deviceWindow, this->deviceExists ? TrackFX_GetOpen(track, deviceIndex) : 0, dump);
	this->deviceExpanded = Collectors::CollectIntValue(ss, "/device/expand", this->deviceExpanded, this->model.deviceExpandedType == 1 ? 1 : 0, dump);

	constexpr int LENGTH = 50;
	std::string strBuffer(LENGTH, 0);
	char* strBufferPointer = &*strBuffer.begin();

	if (this->slowCounter == 0 || dump)
	{
		bool result = this->deviceExists ? TrackFX_GetFXName(track, deviceIndex, strBufferPointer, LENGTH) : false;
		this->deviceName = Collectors::CollectStringValue(ss, "/device/name", this->deviceName, result ? strBuffer : "", dump);
		this->deviceBypass = Collectors::CollectIntValue(ss, "/device/bypass", this->deviceBypass, this->deviceExists && TrackFX_GetEnabled(track, deviceIndex) ? 0 : 1, dump);

		for (int index = 0; index < this->model.DEVICE_BANK_SIZE; index++)
		{
			const int position = this->model.deviceBankOffset + index;
			const int bankDeviceIndex = index + 1;

			std::ostringstream das;
			das << "/device/sibling/" << bankDeviceIndex << "/name";
			result = TrackFX_GetFXName(track, position, strBufferPointer, LENGTH);
			Collectors::CollectStringArrayValue(ss, das.str().c_str(), index, deviceSiblings, result ? strBufferPointer : "", dump);

			std::ostringstream bys;
			bys << "/device/sibling/" << bankDeviceIndex << "/bypass";
			const int isBypassed = TrackFX_GetEnabled(track, position) ? 0 : 1;
			Collectors::CollectIntArrayValue(ss, bys.str().c_str(), index, deviceSiblingsBypass, isBypassed, dump);

			std::ostringstream pys;
			pys << "/device/sibling/" << bankDeviceIndex << "/position";
			Collectors::CollectIntArrayValue(ss, pys.str().c_str(), index, deviceSiblingsPosition, position, dump);

			std::ostringstream sys;
			sys << "/device/sibling/" << bankDeviceIndex << "/selected";
			Collectors::CollectIntArrayValue(ss, sys.str().c_str(), index, deviceSiblingsSelection, deviceIndex == position ? 1 : 0, dump);
		}
	}

	// Cursor device parameters
	const int paramCount = this->deviceExists ? TrackFX_GetNumParams(track, deviceIndex) : 0;
	this->model.deviceParamCount = Collectors::CollectIntValue(ss, "/device/param/count", this->model.deviceParamCount, paramCount, dump);
	for (int index = 0; index < paramCount; index++)
		this->model.GetParameter(index)->CollectData(ss, track, deviceIndex, dump);

	// 
	// First instrument (primary) data
	// 

	const int instrumentIndex = TrackFX_GetInstrument(track);
	const bool instrumentExists = instrumentIndex >= 0;
	this->instrumentExists = Collectors::CollectIntValue(ss, "/primary/exists", this->instrumentExists, instrumentExists, dump);
	this->instrumentPosition = Collectors::CollectIntValue(ss, "/primary/position", this->instrumentPosition, instrumentIndex, dump);
	const bool result = instrumentExists && TrackFX_GetFXName(track, instrumentIndex, strBufferPointer, LENGTH);
	this->instrumentName = Collectors::CollectStringValue(ss, "/primary/name", this->instrumentName, result ? strBuffer : "", dump);

	const int instParamCount = this->instrumentExists ? TrackFX_GetNumParams(track, instrumentIndex) : 0;
	this->instrumentParameterCount = Collectors::CollectIntValue(ss, "/primary/param/count", this->instrumentParameterCount, instParamCount, dump);
	for (int index = 0; index < instParamCount; index++)
		this->model.GetInstrumentParameter(index)->CollectData(ss, track, instrumentIndex, dump);

	// 
	// First ReaEQ data
	// 

	const int eqIndex = TrackFX_GetEQ(track, false);
	this->eqExists = Collectors::CollectIntValue(ss, "/eq/exists", this->eqExists, eqIndex >= 0, dump);
	const int eqParamCount = TrackFX_GetNumParams(track, eqIndex);
	this->model.eqParamCount = Collectors::CollectIntValue(ss, "/eq/param/count", this->model.eqParamCount, eqParamCount, dump);

	if (this->eqExists)
	{
		constexpr int FILTER_TYPE_LENGTH = 40;
		std::string filterType(FILTER_TYPE_LENGTH, 0);
		char* filterTypePointer = &*filterType.begin();

		constexpr int FILTER_ENABLED_LENGTH = 2;
		std::string filterEnabled(FILTER_ENABLED_LENGTH, 0);
		char* filterEnabledPointer = &*filterEnabled.begin();

		for (int index = 0; index < 8; index++)
		{
			std::ostringstream btss;
			btss << "BANDTYPE" << index;
			const std::string bandTypeParam = btss.str();
			std::string bandType = "-1";
			if (TrackFX_GetNamedConfigParm(track, eqIndex, bandTypeParam.c_str(), filterTypePointer, FILTER_TYPE_LENGTH))
			{
				std::ostringstream bess;
				bess << "BANDENABLED" << index;
				const std::string bandEnabledParam = bess.str();
				if (TrackFX_GetNamedConfigParm(track, eqIndex, bandEnabledParam.c_str(), filterEnabledPointer, FILTER_ENABLED_LENGTH))
				{
					if (std::atoi(filterEnabledPointer) > 0)
						bandType = filterType;
				}
			}
			std::ostringstream das;
			das << "/eq/band/" << index;
			this->eqBandTypes.at(index) = Collectors::CollectStringValue(ss, das.str().c_str(), this->eqBandTypes.at(index), bandType, dump);
		}
	}

	for (int index = 0; index < eqParamCount; index++)
	{
		const std::unique_ptr<Parameter>& parameter = this->model.GetEqParameter(index);
		parameter->CollectData(ss, track, eqIndex, dump);
	}

	// Track FX Parameter
	const int trackFxParamCount = CountTCPFXParms(project, track);
	this->model.trackFxParamCount = Collectors::CollectIntValue(ss, "/track/fx/param/count", this->model.trackFxParamCount, trackFxParamCount, dump);
	int fxindexOut = 0;
	int parmidxOut = 0;
	for (int index = 0; index < trackFxParamCount; index++)
	{
		const std::unique_ptr<Parameter>& parameter = this->model.GetTrackFXParameter(index);
		if (GetTCPFXParm(project, track, index, &fxindexOut, &parmidxOut))
			parameter->CollectData(ss, track, fxindexOut, parmidxOut, dump);
		else
			parameter->ClearData(ss, dump);
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
		const std::unique_ptr<Track>& track = this->model.GetTrack(trackIndex);
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

	bool isSelected{ false };
	bool isMuted{ false };
	int channel{ 0 }, pitch{ 0 }, velocity{ 0 };
	double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };

	std::ostringstream notes;

	for (int i = 0; i < noteCount; ++i)
	{
		MIDI_GetNote(take, i, &isSelected, &isMuted, &startppqpos, &endppqpos, &channel, &pitch, &velocity);

		musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
		musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);

		if (this->playPosition >= musicalStart && this->playPosition <= musicalEnd)
			notes << (isSelected ? 1 : 0) << ":" << (isMuted ? 1 : 0) << ":" << musicalStart << ":" << musicalEnd << ":" << channel << ":" << pitch << ":" << velocity << ";";
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
	const double cursorPos = ReaperUtils::GetCursorPosition(project);

	int trackState;
	GetTrackState(master, &trackState);

	this->masterSelected = Collectors::CollectIntValue(ss, "/master/select", this->masterSelected, (trackState & 2) > 0, dump);
	this->masterMute = Collectors::CollectIntValue(ss, "/master/mute", this->masterMute, this->GetMasterMute(master, cursorPos, trackState), dump);
	this->masterSolo = Collectors::CollectIntValue(ss, "/master/solo", this->masterSolo, (trackState & 16) > 0 ? 1 : 0, dump);

	// Master track volume and pan
	const double volDB = this->GetMasterVolume(master, cursorPos);
	this->model.masterVolume = Collectors::CollectDoubleValue(ss, "/master/volume", this->model.masterVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->masterVolumeStr = Collectors::CollectStringValue(ss, "/master/volume/str", this->masterVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);

	const double panVal = this->GetMasterPan(master, cursorPos);
	this->model.masterPan = Collectors::CollectDoubleValue(ss, "/master/pan", this->model.masterPan, (panVal + 1) / 2, dump);
	this->masterPanStr = Collectors::CollectStringValue(ss, "/master/pan/str", this->masterPanStr, Collectors::FormatPan(panVal).c_str(), dump);

	const double peakLeft = Track_GetPeakInfo(master, 0);
	const double peakRight = Track_GetPeakInfo(master, 1);
	this->masterVU = Collectors::CollectDoubleValue(ss, "/master/vu", this->masterVU, ReaperUtils::ValueToVURange((peakLeft + peakRight) / 2.0), dump);
	this->masterVULeft = Collectors::CollectDoubleValue(ss, "/master/vuleft", this->masterVULeft, ReaperUtils::ValueToVURange(peakLeft), dump);
	this->masterVURight = Collectors::CollectDoubleValue(ss, "/master/vuright", this->masterVURight, ReaperUtils::ValueToVURange(peakRight), dump);

	if (this->slowCounter == 0 || dump)
	{
		// Track color
		int red = -1, green = -1, blue = -1;
		// Note: GetTrackColor is not working for the master track
		const int nativeColor = gsl::narrow_cast<int> (GetMediaTrackInfo_Value(master, "I_CUSTOMCOLOR"));
		if (nativeColor != 0)
			ColorFromNative(nativeColor & 0xFEFFFFFF, &red, &green, &blue);
		this->masterColor = Collectors::CollectStringValue(ss, "/master/color", this->masterColor, Collectors::FormatColor(red, green, blue).c_str(), dump);
	}

	// Master FX Parameter
	int fxindexOut = 0;
	int parmidxOut = 0;
	const int masterFxParamCount = CountTCPFXParms(project, master);
	this->model.masterFxParamCount = Collectors::CollectIntValue(ss, "/master/fx/param/count", this->model.masterFxParamCount, masterFxParamCount, dump);
	for (int index = 0; index < masterFxParamCount; index++)
	{
		const std::unique_ptr<Parameter>& parameter = this->model.GetMasterFXParameter(index);
		if (GetTCPFXParm(project, master, index, &fxindexOut, &parmidxOut))
			parameter->CollectData(ss, master, fxindexOut, parmidxOut, dump);
		else
			parameter->ClearData(ss, dump);
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

		const int clipColor = GetDisplayedMediaItemColor(item);
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

	// Did notes change since last call?
	constexpr int HASH_LENGTH = 16;
	std::string nhash(HASH_LENGTH, 0);
	char* nhashPointer = &*nhash.begin();
	if (!MIDI_GetHash(take, true, nhashPointer, HASH_LENGTH) || this->noteHash.compare(nhash) == 0)
		return this->notesStr;
	this->noteHash = nhash;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return notesStrNew;

	bool isSelected{ false };
	bool isMuted{ false };
	int channel{ 0 }, pitch{ 0 }, velocity{ 0 };
	double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };
	double bpm{};
	std::ostringstream notes;
	const double pos = GetMediaItemInfo_Value(item, "D_POSITION");

	for (int i = 0; i < noteCount; ++i)
	{
		MIDI_GetNote(take, i, &isSelected, &isMuted, &startppqpos, &endppqpos, &channel, &pitch, &velocity);

		musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
		musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);
		musicalStart -= pos;
		musicalEnd -= pos;

		TimeMap_GetTimeSigAtTime(project, startppqpos, nullptr, nullptr, &bpm);
		musicalStart = bpm * musicalStart / 60.0;
		TimeMap_GetTimeSigAtTime(project, endppqpos, nullptr, nullptr, &bpm);
		musicalEnd = bpm * musicalEnd / 60.0;
		notes << (isSelected ? 1 : 0) << ":" << (isMuted ? 1 : 0) << ":" << musicalStart << ":" << musicalEnd << ":" << channel << ":" << pitch << ":" << velocity << ";";
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
	const int deviceIndex = this->model.GetDeviceSelection();

	// Get the filename which contains the available presets of the device
	constexpr int BUFFER_LENGTH = 1024;
	std::string strBuffer(BUFFER_LENGTH, 0);
	char* strBufferPointer = &*strBuffer.begin();
	TrackFX_GetUserPresetFilename(track, deviceIndex, strBufferPointer, BUFFER_LENGTH);
	this->devicePresetFilename = Collectors::CollectStringValue(ss, "/browser/presetsfile", this->devicePresetFilename, strBufferPointer, dump);

	// Get the current preset index and name
	TrackFX_GetPreset(track, deviceIndex, strBufferPointer, BUFFER_LENGTH);
	this->devicePresetName = Collectors::CollectStringValue(ss, "/browser/selected/name", this->devicePresetName, strBufferPointer, dump);
	int numberOfPresets;
	const int selectedIndex = TrackFX_GetPresetIndex(track, deviceIndex, &numberOfPresets);
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
	if (this->slowCounter == 0 || dump)
	{
		const std::vector<int> markers = Marker::GetMarkers(project);
		const int count = gsl::narrow_cast<int> (markers.size());
		this->model.markerCount = Collectors::CollectIntValue(ss, "/marker/count", this->model.markerCount, count, dump);
		for (int index = 0; index < count; index++)
		{
			const std::unique_ptr<Marker>& marker = this->model.GetMarker(index);
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

	std::ostringstream clipStr;

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
		char buf[2048] = {};

		const int itemCount = CountTrackMediaItems(mediaTrack);

		int realCount{ 0 };
		std::ostringstream allClipStr;

		for (int i = 0; i < itemCount; i++)
		{
			MediaItem* item = GetTrackMediaItem(mediaTrack, i);
			if (item == nullptr)
				continue;
			MediaItem_Take* take = GetActiveTake(item);
			if (take == nullptr)
				continue;

			// Format track index, clip index, name, selected state and color in one string
			DISABLE_WARNING_ARRAY_POINTER_DECAY
			if (GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false))
			{
				std::string s{ buf };
				std::replace(s.begin(), s.end(), ';', ' ');
				allClipStr << s;
			}
			else
			{
				ReaDebug() << "ERROR: Could not read name from clip " << i << " on track " << (trackIndex + 1);
				allClipStr << "Unknown";
			}

			const bool isSelected = IsMediaItemSelected(item);
			allClipStr << ";" << isSelected;

			const int clipColor = GetDisplayedMediaItemColor(item);
			ColorFromNative(clipColor & 0xFEFFFFFF, &red, &green, &blue);
			allClipStr << ";" << Collectors::FormatColor(red, green, blue).c_str() << ";";

			realCount++;
		}
		clipStr << trackIndex << ";" << realCount << ";" << allClipStr.str();

		trackIndex++;
	}
	this->formattedClips = Collectors::CollectStringValue(ss, "/clip/all", this->formattedClips, clipStr.str().c_str(), dump);

	// Collect "scenes", use Region markers
	const std::vector<int> regions = Marker::GetRegions(project);
	count = gsl::narrow_cast<int>(regions.size());
	this->model.sceneCount = Collectors::CollectIntValue(ss, "/scene/count", this->model.sceneCount, count, dump);
	for (int index = 0; index < count; index++)
	{
		const std::unique_ptr<Marker>& scene = this->model.GetRegion(regions.at(index));
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
	// Don't include the master track here
	MediaTrack* const track = GetSelectedTrack(project, 0);

	// Midi note repeat plugin is on track?
	const int position = track ? TrackFX_AddByName(track, NoteRepeatProcessor::MIDI_ARP_PLUGIN, true, 0) : -1;
	const int inputPosition = 0x1000000 + position;

	const int repeatActive = position > -1 && TrackFX_GetEnabled(track, inputPosition) ? 1 : 0;
	this->repeatActive = Collectors::CollectIntValue(ss, "/noterepeat/active", this->repeatActive, repeatActive, dump);

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


double DataCollector::GetMasterVolume(MediaTrack* master, double position) const noexcept
{
	if (GetMediaTrackInfo_Value(master, "I_AUTOMODE") > 0)
	{
		TrackEnvelope* envelope = GetTrackEnvelopeByName(master, "Volume");
		if (envelope != nullptr)
			return ReaperUtils::ValueToDB(ReaperUtils::GetEnvelopeValueAtPosition(envelope, position));
	}
	return ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(master, "D_VOL"));
}


double DataCollector::GetMasterPan(MediaTrack* master, double position) const noexcept
{
	if (GetMediaTrackInfo_Value(master, "I_AUTOMODE") > 0)
	{
		TrackEnvelope* envelope = GetTrackEnvelopeByName(master, "Pan");
		if (envelope != nullptr)
		{
			// Higher values are left!
			return -1 * ReaperUtils::GetEnvelopeValueAtPosition(envelope, position);
		}
	}
	return GetMediaTrackInfo_Value(master, "D_PAN");
}


int DataCollector::GetMasterMute(MediaTrack* master, double position, int trackState) const noexcept
{
	if (GetMediaTrackInfo_Value(master, "I_AUTOMODE") > 0)
	{
		TrackEnvelope* envelope = GetTrackEnvelopeByName(master, "Mute");
		if (envelope != nullptr)
		{
			// The envelope is inverted!
			return ReaperUtils::GetEnvelopeValueAtPosition(envelope, position) > 0 ? 0 : 1;
		}
	}
	return (trackState & 8) > 0 ? 1 : 0;
}


/**
 * Collect the Groove data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectGrooveData(std::ostringstream& ss, ReaProject* project, const bool& dump)
{
	double divisionInOutOptional;
	int swingmodeInOutOptional;
	double swingamtInOutOptional;
	GetSetProjectGrid(project, false, &divisionInOutOptional, &swingmodeInOutOptional, &swingamtInOutOptional);

	this->swingActive = Collectors::CollectIntValue(ss, "/groove/active", this->swingActive, swingmodeInOutOptional, dump);
	this->swingAmount = Collectors::CollectDoubleValue(ss, "/groove/amount", this->swingAmount, swingamtInOutOptional, dump);
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
	const std::lock_guard<std::mutex> lock(this->delayMutex);
	bool result = true;
	const std::map<std::string, long long>::iterator it = this->delayUpdateMap.find(processor);
	if (it != this->delayUpdateMap.end())
	{
		const long long oldValue = it->second;
		const long long millis = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
		if (millis - oldValue < DELAY)
			result = false;
		else
			this->delayUpdateMap.erase(processor);
	}
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


void DataCollector::replaceCommaWithDot(std::string& str)
{
	std::replace(str.begin(), str.end(), ',', '.');
}
