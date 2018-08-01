// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "DataCollector.h"


/**
 * Constructor.
 */
DataCollector::DataCollector(Model &aModel) :
	model(aModel),
	trackExists(aModel.trackBankSize, 0),
	trackNumber(aModel.trackBankSize, 0),
	trackName(aModel.trackBankSize, ""),
	trackType(aModel.trackBankSize, ""),
	trackSelected(aModel.trackBankSize, 0),
	trackMute(aModel.trackBankSize, 0),
	trackSolo(aModel.trackBankSize, 0),
	trackRecArmed(aModel.trackBankSize, 0),
	trackActive(aModel.trackBankSize, 0),
	trackMonitor(aModel.trackBankSize, 0),
	trackAutoMonitor(aModel.trackBankSize, 0),
	trackColor(aModel.trackBankSize, ""),
	trackVolumeStr(aModel.trackBankSize, ""),
	trackPanStr(aModel.trackBankSize, ""),
	trackVULeft(aModel.trackBankSize, 0),
	trackVURight(aModel.trackBankSize, 0),
	trackAutoMode(aModel.trackBankSize, 0),
	trackSendName(aModel.trackBankSize, std::vector<std::string>(aModel.sendBankSize, "")),
	trackSendVolumeStr(aModel.trackBankSize, std::vector<std::string>(aModel.sendBankSize, "")),
	trackRepeatActive(aModel.trackBankSize, 0),
	trackRepeatNoteLength(aModel.trackBankSize, 0),
	deviceSiblings(aModel.deviceBankSize, ""),
	deviceParamName(aModel.parameterBankSize, ""),
	deviceParamValue(aModel.parameterBankSize, 0),
	deviceParamValueStr(aModel.parameterBankSize, ""),
	devicePresetsStr(128, ""),
	markerExists(aModel.markerBankSize, 0),
	markerNumber(aModel.markerBankSize, 0),
	markerName(aModel.markerBankSize, ""),
	markerColor(aModel.markerBankSize, "")
{
	this->trackStateChunk = std::make_unique<char []> (BUFFER_SIZE);
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
std::string DataCollector::CollectData(const bool &dump)
{
	std::stringstream ss;
	ReaProject *project = this->model.GetProject();

	CollectProjectData(ss, project, dump);
	CollectTransportData(ss, project, dump);
	CollectTrackData(ss, project, dump);
	CollectDeviceData(ss, project, dump);
	CollectMasterTrackData(ss, project, dump);
	CollectClipData(ss, project, dump);
	CollectBrowserData(ss, project, dump);
	CollectMarkerData(ss, project, dump);

	return ss.str();
}


/**
 * Collect the (changed) project data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectProjectData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	char newProjectName[20];
	GetProjectName(project, newProjectName, 20);
	this->projectName = CollectStringValue(ss, "/project/name", projectName, newProjectName, dump);
	this->projectEngine = CollectIntValue(ss, "/project/engine", projectEngine, Audio_IsRunning(), dump);
}


/**
 * Collect the (changed) transport data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectTransportData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	// Transport states
	const int playState = GetPlayStateEx(project);
	this->play = CollectIntValue(ss, "/play", this->play, (playState & 1) > 0, dump);
	this->record = CollectIntValue(ss, "/record", this->record, (playState & 4) > 0, dump);
	this->repeat = CollectIntValue(ss, "/repeat", this->repeat, GetSetRepeat(-1), dump);
	this->metronome = CollectIntValue(ss, "/click", this->metronome, GetToggleCommandState(40364), dump);

	// The tempo
	this->tempo = CollectDoubleValue(ss, "/tempo", this->tempo, Master_GetTempo(), dump);

	// Get the time signature at the current play position
	double cursorPos = GetCursorPositionEx(project);
	int timesig;
	int denomOut;
	double startBPM;
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->globalTimesig = CollectIntValue(ss, "/numerator", this->globalTimesig, timesig, dump);
	this->globalDenomOut = CollectIntValue(ss, "/denominator", this->globalDenomOut, denomOut, dump);

	cursorPos = this->play ? GetPlayPositionEx(project) : GetCursorPositionEx(project);
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->playPosition = CollectDoubleValue(ss, "/time", this->playPosition, startBPM * cursorPos / 60, dump);
	char timeStr[20];
	format_timestr(cursorPos, timeStr, 20);
	this->strPlayPosition = CollectStringValue(ss, "/time/str", this->strPlayPosition, timeStr, dump);
	format_timestr_pos(cursorPos, timeStr, 20, 2);
	this->strBeatPosition = CollectStringValue(ss, "/beat", this->strBeatPosition, timeStr, dump);

	this->prerollClick = CollectIntValue(ss, "/prerollClick", this->prerollClick, GetToggleCommandState(41819), dump);
}


/**
 * Collect the (changed) device data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectDeviceData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + this->model.trackSelection);

	const int deviceIndex = this->model.deviceBankOffset + this->model.deviceSelected;
	int bankDeviceIndex = 1;
	this->model.deviceCount = CollectIntValue(ss, "/device/count", this->model.deviceCount, TrackFX_GetCount(track), dump);
	this->deviceExists = CollectIntValue(ss, "/device/exists", this->deviceExists, deviceIndex < this->model.deviceCount ? 1 : 0, dump);
	this->devicePosition = CollectIntValue(ss, "/device/position", this->devicePosition, deviceIndex, dump);
	this->deviceWindow = CollectIntValue(ss, "/device/window", this->deviceWindow, TrackFX_GetOpen(track, deviceIndex), dump);
	this->deviceExpanded = CollectIntValue(ss, "/device/expand", this->deviceExpanded, this->model.deviceExpandedType == 1, dump);

	const int LENGTH = 20;
	char name[LENGTH];
	bool result = TrackFX_GetFXName(track, deviceIndex, name, LENGTH);
	this->deviceName = CollectStringValue(ss, "/device/name", this->deviceName, result ? name : "", dump);
	this->deviceBypass = CollectIntValue(ss, "/device/bypass", this->deviceBypass, TrackFX_GetEnabled(track, deviceIndex) ? 0 : 1, dump);

	for (int index = 0; index < this->model.deviceBankSize; index++)
	{
		std::stringstream das;
		das << "/device/sibling/" << bankDeviceIndex << "/name";
		std::string deviceAddress = das.str();
		result = TrackFX_GetFXName(track, this->model.deviceBankOffset + index, name, LENGTH);
		CollectStringArrayValue(ss, das.str().c_str(), index, deviceSiblings, result ? name : "", dump);
		bankDeviceIndex++;
	}

	const int paramCount = TrackFX_GetNumParams(track, deviceIndex);
	this->model.deviceParamCount = CollectIntValue(ss, "/device/param/count", this->model.deviceParamCount, paramCount, dump);
	this->model.deviceParamBankSelected = CollectIntValue(ss, "/device/param/bank/selected", this->model.deviceParamBankSelected, this->model.deviceParamBankSelectedTemp, dump);

	int paramIndex = this->model.deviceParamBankSelected * this->model.parameterBankSize;
	for (int index = 0; index < this->model.parameterBankSize; index++)
	{
		std::stringstream das;
		das << "/device/param/" << index + 1 << "/";
		std::string paramAddress = das.str();

		result = TrackFX_GetParamName(track, deviceIndex, paramIndex, name, LENGTH);
		CollectStringArrayValue(ss, (paramAddress + "name").c_str(), index, this->deviceParamName, result ? name : "", dump);
		const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, paramIndex);
		CollectDoubleArrayValue(ss, (paramAddress + "value").c_str(), index, this->deviceParamValue, paramValue, dump);
		result = TrackFX_FormatParamValueNormalized(track, deviceIndex, paramIndex, paramValue, name, LENGTH);
		CollectStringArrayValue(ss, (paramAddress + "value/str").c_str(), index, this->deviceParamValueStr, result ? name : "", dump);

		paramIndex += 1;
	}
}

/**
 * Collect the (changed) track data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectTrackData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	this->AdjustTrackBank(project);

	int trackIndex = this->model.trackBankOffset;
	int bankTrackIndex = 1;
	this->model.trackCount = CollectIntValue(ss, "/track/count", this->model.trackCount, CountTracks(project), dump);

	const int LENGTH = 20;
	char name[LENGTH];

	for (int index = 0; index < this->model.trackBankSize; index++)
	{
		std::stringstream das;
		das << "/track/" << bankTrackIndex << "/";
		std::string trackAddress = das.str();

		// Track exists flag and number of tracks
		CollectIntArrayValue(ss, (trackAddress + "exists").c_str(), index, this->trackExists, trackIndex < this->model.trackCount ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "number").c_str(), index, this->trackNumber, trackIndex, dump);

		// Track name
		MediaTrack *track = GetTrack(project, trackIndex);
		bool result = track != nullptr && GetTrackName(track, name, LENGTH);
		CollectStringArrayValue(ss, (trackAddress + "name").c_str(), index, this->trackName, result ? name : "", dump);

		// Track type (GROUP or HYBRID), select, mute, solo, recarm and monitor states
		int trackState{};
		if (track != nullptr)
			GetTrackState(track, &trackState);
		CollectStringArrayValue(ss, (trackAddress + "type").c_str(), index, this->trackType, (trackState & 1) > 0 ? "GROUP" : "HYBRID", dump);
		const int selected = (trackState & 2) > 0 ? 1 : 0;
		if (trackIndex < this->model.trackCount && selected)
			this->model.trackSelection = index;

		CollectIntArrayValue(ss, (trackAddress + "select").c_str(), index, this->trackSelected, selected, dump);
		CollectIntArrayValue(ss, (trackAddress + "mute").c_str(), index, this->trackMute, (trackState & 8) > 0 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "solo").c_str(), index, this->trackSolo, (trackState & 16) > 0 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "recarm").c_str(), index, this->trackRecArmed, (trackState & 64) > 0 ? 1 : 0, dump);
		// Uses "lock track" as active indication
		CollectIntArrayValue(ss, (trackAddress + "active").c_str(), index, this->trackActive, track != nullptr && GetTrackLockState(track) ? 0 : 1, dump);

		const double monitor = track != nullptr ? GetMediaTrackInfo_Value(track, "I_RECMON") : 0;
		CollectIntArrayValue(ss, (trackAddress + "monitor").c_str(), index, this->trackMonitor, monitor == 1 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "autoMonitor").c_str(), index, this->trackAutoMonitor, monitor == 2 ? 1 : 0, dump);

		// Track color
		int red = 0, green = 0, blue = 0;
		if (track != nullptr)
			ColorFromNative(GetTrackColor(track) & 0xFEFFFFFF, &red, &green, &blue);
		CollectStringArrayValue(ss, (trackAddress + "color").c_str(), index, this->trackColor, FormatColor(red, green, blue).c_str(), dump);

		// Track volume and pan
		double volDB = track != nullptr ? this->model.ValueToDB(GetMediaTrackInfo_Value(track, "D_VOL")) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "volume").c_str(), index, this->model.trackVolume, DB2SLIDER(volDB) / 1000.0, dump);
		CollectStringArrayValue(ss, (trackAddress + "volume/str").c_str(), index, this->trackVolumeStr, FormatDB(volDB).c_str(), dump);
		const double panVal = track != nullptr ? GetMediaTrackInfo_Value(track, "D_PAN") : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "pan").c_str(), index, this->model.trackPan, (panVal + 1) / 2, dump);
		CollectStringArrayValue(ss, (trackAddress + "pan/str").c_str(), index, this->trackPanStr, FormatPan(panVal).c_str(), dump);

		// VU and automation mode
		double peak = track != nullptr ? Track_GetPeakInfo(track, 0) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "vuleft").c_str(), index, this->trackVULeft, DB2SLIDER(this->model.ValueToDB(peak)) / 1000.0, dump);
		peak = track != nullptr ? Track_GetPeakInfo(track, 1) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "vuright").c_str(), index, this->trackVURight, DB2SLIDER(this->model.ValueToDB(peak)) / 1000.0, dump);
		const double automode = track != nullptr ? GetMediaTrackInfo_Value(track, "I_AUTOMODE") : 0;
		CollectIntArrayValue(ss, (trackAddress + "automode").c_str(), index, this->trackAutoMode, static_cast<int>(automode), dump);

		// Sends
		const int numSends = track != nullptr ? GetTrackNumSends(track, 0) : 0;
		for (int sendCounter = 0; sendCounter < this->model.sendBankSize; sendCounter++)
		{
			std::stringstream stream;
			stream << trackAddress << "send/" << sendCounter + 1 << "/";
			std::string sendAddress = stream.str();
			if (sendCounter < numSends)
			{
				result = GetTrackSendName(track, sendCounter, name, LENGTH);
				CollectStringArrayValue(ss, (sendAddress + "name").c_str(), index, this->trackSendName.at(sendCounter), result ? name : "", dump);
				volDB = this->model.ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
				CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), index, this->model.trackSendVolume.at(sendCounter), DB2SLIDER(volDB) / 1000.0, dump);
				CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), index, this->trackSendVolumeStr.at(sendCounter), FormatDB(volDB).c_str(), dump);
			}
			else
			{
				CollectStringArrayValue(ss, (sendAddress + "name").c_str(), index, this->trackSendName.at(sendCounter), "", dump);
				CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), index, this->model.trackSendVolume.at(sendCounter), 0, dump);
				CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), index, this->trackSendVolumeStr.at(sendCounter), "", dump);
			}
		}

		// Midi note repeat plugin is on track?
		const int position = track != nullptr ? TrackFX_AddByName(track, "midi_note_repeater", 1, 0) : -1;
		const int repeatActive = position > -1 && TrackFX_GetEnabled(track, 0x1000000 + position) ? 1 : 0;
		double minVal{}, maxVal{};
		const int repeatNoteLength = position > -1 ? (int)TrackFX_GetParam(track, 0x1000000 + position, 0, &minVal, &maxVal) : 1;
		CollectIntArrayValue(ss, (trackAddress + "repeatActive").c_str(), index, this->trackRepeatActive, repeatActive ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "noterepeatlength").c_str(), index, this->trackRepeatNoteLength, repeatNoteLength, dump);

		trackIndex += 1;
		bankTrackIndex += 1;
	}
}

/**
 * Collect the (changed) master track data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectMasterTrackData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	MediaTrack *master = GetMasterTrack(project);

	int trackState;
	GetTrackState(master, &trackState);

	CollectIntValue(ss, "/master/select", this->masterSelected, (trackState & 2) > 0, dump);
	CollectIntValue(ss, "/master/mute", this->masterMute, (trackState & 8) > 0 ? 1 : 0, dump);
	CollectIntValue(ss, "/master/solo", this->masterSolo, (trackState & 16) > 0 ? 1 : 0, dump);

	// Master track volume and pan
	const double volDB = this->model.ValueToDB(GetMediaTrackInfo_Value(master, "D_VOL"));
	this->model.masterVolume = CollectDoubleValue(ss, "/master/volume", this->model.masterVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->masterVolumeStr = CollectStringValue(ss, "/master/volume/str", this->masterVolumeStr, FormatDB(volDB).c_str(), dump);

	const double panVal = GetMediaTrackInfo_Value(master, "D_PAN");
	this->model.masterPan = CollectDoubleValue(ss, "/master/pan", this->model.masterPan, (panVal + 1) / 2, dump);
	this->masterPanStr = CollectStringValue(ss, "/master/pan/str", this->masterPanStr, FormatPan(panVal).c_str(), dump);

	this->masterVULeft = CollectDoubleValue(ss, "/master/vuleft", this->masterVULeft, DB2SLIDER(this->model.ValueToDB(Track_GetPeakInfo(master, 0))) / 1000.0, dump);
	this->masterVURight = CollectDoubleValue(ss, "/master/vuright", this->masterVURight, DB2SLIDER(this->model.ValueToDB(Track_GetPeakInfo(master, 1))) / 1000.0, dump);
}


/**
 * Collect the (changed) clip data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectClipData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	// Get the selected media item if any and calculate the items start and end
	double musicalStart{ -1 };
	double musicalEnd{ -1 };
	int timesig{};
	int denomOut{};
	double startBPM{};
	double endBPM{};
	const int count = CountSelectedMediaItems(project);
	if (count > 0)
	{
		MediaItem *item = GetSelectedMediaItem(project, 0);
		const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		const double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
		TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &endBPM);
		musicalStart = startBPM * itemStart / 60;
		musicalEnd = endBPM * itemEnd / 60;
	}
	this->clipMusicalStart = CollectDoubleValue(ss, "/clip/start", this->clipMusicalStart, musicalStart, dump);
	this->clipMusicalEnd = CollectDoubleValue(ss, "/clip/end", this->clipMusicalEnd, musicalEnd, dump);

	// Get the loop start and end if any
	double loopStart{};
	double loopEnd{};
	GetSet_LoopTimeRange2(project, 0, 0, &loopStart, &loopEnd, 0);
	TimeMap_GetTimeSigAtTime(project, loopStart, &timesig, &denomOut, &startBPM);
	TimeMap_GetTimeSigAtTime(project, loopEnd, &timesig, &denomOut, &endBPM);
	double musicalLoopStart = -1;
	double musicalLoopEnd = -1;
	if (loopStart != 0 || loopEnd != 0)
	{
		musicalLoopStart = startBPM * loopStart / 60;
		musicalLoopEnd = endBPM * loopEnd / 60;
	}
	this->globalMusicalLoopStart = CollectDoubleValue(ss, "/clip/loopStart", this->globalMusicalLoopStart, musicalLoopStart, dump);
	this->globalMusicalLoopEnd = CollectDoubleValue(ss, "/clip/loopEnd", this->globalMusicalLoopEnd, musicalLoopEnd, dump);
}


/**
 * Collect the (changed) browser data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectBrowserData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	MediaTrack *track = GetTrack(project, this->model.trackBankOffset + this->model.trackSelection);
	const int sel = this->model.deviceBankOffset + this->model.deviceSelected;
	LoadDevicePresetFile(ss, track, sel, dump);

	const int LENGTH = 20;
	char presetname[LENGTH];
	TrackFX_GetPreset(track, sel, presetname, LENGTH);
	this->devicePresetName = CollectStringValue(ss, "/browser/selected/name", this->devicePresetName, presetname, dump);
	int numberOfPresets;
	const int selectedIndex = TrackFX_GetPresetIndex(track, this->model.deviceBankOffset + this->model.deviceSelected, &numberOfPresets);
	this->devicePresetIndex = CollectIntValue(ss, "/browser/selected/index", this->devicePresetIndex, selectedIndex, dump);
}


/**
* Collect the (changed) marker data.
*
* @param ss The stream where to append the formatted data
* @param project The current Reaper project
* @param dump If true all data is collected not only the changed one since the last call
*/
void DataCollector::CollectMarkerData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	int markerIndex = this->model.markerBankOffset;
	int bankMarkerIndex = 1;

	this->model.markerCount = CollectIntValue(ss, "/marker/count", this->model.markerCount, CountProjectMarkers(project, nullptr, nullptr), dump);

	const char* name;
	bool isRegion;
	double markerPos;
	double regionEnd;
	int markerRegionIndexNumber;
	int markerColor;

	for (int index = 0; index < this->model.markerBankSize; index++)
	{
		std::stringstream das;
		das << "/marker/" << bankMarkerIndex << "/";
		std::string markerAddress = das.str();

		// Marker exists flag and number of markers
		const bool exists = markerIndex < this->model.markerCount ? 1 : 0;
		CollectIntArrayValue(ss, (markerAddress + "exists").c_str(), index, this->markerExists, exists, dump);
		CollectIntArrayValue(ss, (markerAddress + "number").c_str(), index, this->markerNumber, markerIndex, dump);

		int result = exists ? EnumProjectMarkers3(project, index, &isRegion, &markerPos, &regionEnd, &name, &markerRegionIndexNumber, &markerColor) : 0;

		// Marker name
		CollectStringArrayValue(ss, (markerAddress + "name").c_str(), index, this->markerName, result ? name : "", dump);

		// Marker color
		int red = 0, green = 0, blue = 0;
		if (exists)
			ColorFromNative(markerColor & 0xFEFFFFFF, &red, &green, &blue);
		CollectStringArrayValue(ss, (markerAddress + "color").c_str(), index, this->markerColor, FormatColor(red, green, blue).c_str(), dump);

		markerIndex += 1;
		bankMarkerIndex += 1;
	}
}


void DataCollector::LoadDevicePresetFile(std::stringstream &ss, MediaTrack *track, int fx, const bool &dump)
{
	const int LENGTH = 1024;
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

				std::stringstream das;
				das << "/browser/result/" << counter + 1 << "/name";
				CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, strip.c_str(), dump);

				counter += 1;
			}
		}
		file.close();

		while (counter < 128)
		{
			std::stringstream das;
			das << "/browser/result/" << counter + 1 << "/name";
			CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, "", dump);
			counter += 1;
		}
	}
	catch (std::ios_base::failure &ex)
	{
		(void)ex;
		// File does not exist
		return;
	}
}


void DataCollector::AdjustTrackBank(ReaProject *project)
{
	MediaTrack *track = GetSelectedTrack(project, 0);
	if (track == nullptr)
		return;
	const int trackIdx = CSurf_TrackToID(track, false) - 1;
	if (trackIdx >= 0)
		this->model.trackBankOffset = static_cast<int>(std::floor(trackIdx / this->model.trackBankSize) * this->model.trackBankSize);
}


int DataCollector::GetTrackLockState(MediaTrack *track)
{
	// Critical error detected c0000374 - GetTrackStateChunk currently not usable
	//if (!GetTrackStateChunk(track, this->trackStateChunk.get(), BUFFER_SIZE, false))
	//	return 0;
	//std::cmatch result;
	//if (!std::regex_search(this->trackStateChunk.get(), result, this->trackLockPattern))
	//	return 0;
	//std::string value = result.str(1);
	//return std::atoi(value.c_str());
	return 0;
}


const char *DataCollector::CollectStringValue(std::stringstream &ss, const char *command, std::string currentValue, const char *newValue, const bool &dump) const
{
	if ((newValue && std::strcmp(currentValue.c_str(), newValue) != 0) || dump)
		ss << command << " " << newValue << "\n";
	return newValue;
}


int DataCollector::CollectIntValue(std::stringstream &ss, const char *command, int currentValue, const int newValue, const bool &dump) const
{
	if (currentValue != newValue || dump)
		ss << command << " " << newValue << "\n";
	return newValue;
}


double DataCollector::CollectDoubleValue(std::stringstream &ss, const char *command, double currentValue, const double newValue, const bool &dump) const
{
	if (fabs(currentValue - newValue) > 0.0000000001 || dump)
		ss << command << " " << newValue << "\n";
	return newValue;
}


void DataCollector::CollectStringArrayValue(std::stringstream &ss, const char *command, int index, std::vector<std::string> &currentValues, const char *newValue, const bool &dump) const
{
	if ((newValue && std::strcmp(currentValues.at(index).c_str(), newValue) != 0) || dump)
	{
		if (newValue == nullptr)
		{
			ss << command << " " << "" << "\n";
			currentValues.at(index).assign("");
			return;
		}
		ss << command << " " << newValue << "\n";
		currentValues.at(index).assign(newValue);
	}
}


void DataCollector::CollectDoubleArrayValue(std::stringstream &ss, const char *command, int index, std::vector<double> &currentValues, double newValue, const bool &dump) const
{
	if (std::fabs(currentValues.at(index) - newValue) > 0.0000000001 || dump)
	{
		ss << command << " " << newValue << "\n";
		currentValues.at(index) = newValue;
	}
}


void DataCollector::CollectIntArrayValue(std::stringstream &ss, const char *command, int index, std::vector<int> &currentValues, int newValue, const bool &dump) const
{
	if (currentValues.at(index) != newValue || dump)
	{
		ss << command << " " << newValue << "\n";
		currentValues.at(index) = newValue;
	}
}


std::string DataCollector::FormatColor(int red, int green, int blue) const
{
	std::stringstream ss;
	ss << red << " " << green << " " << blue;
	return ss.str();
}


std::string DataCollector::FormatDB(double value) const
{
	if (value == -150)
		return "-inf dB";
	std::stringstream stream;
	stream << std::fixed << std::setprecision(1);
	if (value >= 0)
		stream << "+";
	stream << value << " dB";
	return stream.str();
}


std::string DataCollector::FormatPan(double value) const
{
	if (abs(value) < 0.001)
		return "C";
	std::stringstream stream;
	if (value < 0)
		stream << static_cast<int>(value * -100) << "L";
	else
		stream << static_cast<int>(value * 100) << "R";
	return stream.str();
}
