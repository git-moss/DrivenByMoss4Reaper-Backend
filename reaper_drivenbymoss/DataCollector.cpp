// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>
#include <iomanip>

#include "DataCollector.h"


/**
 * Constructor.
 */
DataCollector::DataCollector(Model &aModel) :
	model(aModel),
	trackExists(trackBankSize, 0),
	trackNumber(trackBankSize, 0),
	trackName(trackBankSize, ""),
	trackType(trackBankSize, ""),
	trackSelected(trackBankSize, 0),
	trackMute(trackBankSize, 0),
	trackSolo(trackBankSize, 0),
	trackRecArmed(trackBankSize, 0),
	trackActive(trackBankSize, 0),
	trackMonitor(trackBankSize, 0),
	trackAutoMonitor(trackBankSize, 0),
	trackColor(trackBankSize, ""),
	trackVolume(trackBankSize, 0),
	trackVolumeStr(trackBankSize, ""),
	trackPan(trackBankSize, 0),
	trackPanStr(trackBankSize, ""),
	trackVULeft(trackBankSize, 0),
	trackVURight(trackBankSize, 0),
	trackAutoMode(trackBankSize, 0),
	trackSendName(trackBankSize, std::vector<std::string>(sendBankSize, "")),
	trackSendVolume(trackBankSize, std::vector<double>(sendBankSize, 0)),
	trackSendVolumeStr(trackBankSize, std::vector<std::string>(sendBankSize, "")),
	trackRepeatActive(trackBankSize, 0),
	trackRepeatNoteLength(trackBankSize, 0),
	deviceSiblings(deviceBankSize, ""),
	deviceParamName(parameterBankSize, ""),
	deviceParamValue(parameterBankSize, 0),
	deviceParamValueStr(parameterBankSize, "")
{
	// Intentionally empty
}


/**
 * Destructor.
 */
DataCollector::~DataCollector()
{
	// Intentionally empty
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
	iniPath = CollectStringValue(ss, "/inipath", iniPath, GetResourcePath(), dump);

	ReaProject *project = this->GetProject();
	CollectProjectData(ss, project, dump);
	CollectTransportData(ss, project, dump);
	CollectTrackData(ss, project, dump);
	CollectDeviceData(ss, project, dump);
	CollectMasterTrackData(ss, project, dump);
	CollectGrooveData(ss, project, dump);
	CollectClipData(ss, project, dump);

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
	projectName = CollectStringValue(ss, "/project/name", projectName, newProjectName, dump);
	projectEngine = CollectIntValue(ss, "/project/engine", projectEngine, Audio_IsRunning(), dump);
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
	int playState = GetPlayStateEx(project);
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

	cursorPos = GetPlayPositionEx(project);
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->playPosition = CollectDoubleValue(ss, "/time", this->playPosition, startBPM * cursorPos / 60, dump);
	char timeStr[20];
	format_timestr(cursorPos, timeStr, 20);
	this->strPlayPosition = CollectStringValue(ss, "/time/str", this->strPlayPosition, timeStr, dump);

	if (APIExists("SNM_GetDoubleConfigVar"))
	{
		// TODO This function just hangs...
		// int newPrerollMeasures = (int) SNM_GetDoubleConfigVar("prerollmeas", 2);
		// this->prerollMeasures = CollectIntValue(ss, "/preroll", this->prerollMeasures, newPrerollMeasures, dump);
	}
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
	MediaTrack *track = GetTrack(project, this->trackBankOffset + this->trackSelection);

	const int deviceIndex = this->deviceBankOffset + this->model.deviceSelected;
	int bankDeviceIndex = 1;
	this->deviceCount = CollectIntValue(ss, "/device/count", this->deviceCount, TrackFX_GetCount(track), dump);
	this->deviceExists = CollectIntValue(ss, "/device/exists", this->deviceExists, deviceIndex < this->deviceCount ? 1 : 0, dump);
	this->devicePosition = CollectIntValue(ss, "/device/position", this->devicePosition, deviceIndex, dump);
	this->deviceWindow = CollectIntValue(ss, "/device/window", this->deviceWindow, TrackFX_GetOpen(track, deviceIndex), dump);
	this->deviceExpanded = CollectIntValue(ss, "/device/expand", this->deviceExpanded, this->deviceExpandedTypeTemp == 1, dump);
	this->deviceExpandedType = this->deviceExpandedTypeTemp;

	const int LENGTH = 20;
	char name[LENGTH];
	bool result = TrackFX_GetFXName(track, deviceIndex, name, LENGTH);
	this->deviceName = CollectStringValue(ss, "/device/name", this->deviceName, result ? name : "", dump);
	this->deviceBypass = CollectIntValue(ss, "/device/bypass", this->deviceBypass, TrackFX_GetEnabled(track, deviceIndex) ? 0 : 1, dump);

	for (int index = 0; index < this->deviceBankSize; index++)
	{
		std::stringstream das;
		das << "/device/sibling/" << bankDeviceIndex << "/name";
		std::string deviceAddress = das.str();
		bool result = TrackFX_GetFXName(track, this->deviceBankOffset + index, name, LENGTH);
		CollectStringArrayValue(ss, das.str().c_str(), index, deviceSiblings, result ? name : "", dump);
		bankDeviceIndex++;
	}

	int paramCount = TrackFX_GetNumParams(track, deviceIndex);
	this->deviceParamCount = CollectIntValue(ss, "/device/param/count", this->deviceParamCount, paramCount, dump);
	this->model.deviceParamBankSelected = CollectIntValue(ss, "/device/param/bank/selected", this->model.deviceParamBankSelected, this->model.deviceParamBankSelectedTemp, dump);

	int paramIndex = this->model.deviceParamBankSelected * this->parameterBankSize;
	for (int index = 0; index < parameterBankSize; index++)
	{
		std::stringstream das;
		das << "/device/param/" << index + 1 << "/";
		std::string paramAddress = das.str();

		result = TrackFX_GetParamName(track, deviceIndex, paramIndex, name, LENGTH);
		CollectStringArrayValue(ss, (paramAddress + "name").c_str(), index, this->deviceParamName, result ? name : "", dump);
		double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, paramIndex);
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

	int trackIndex = this->trackBankOffset;
	int bankTrackIndex = 1;
	this->trackCount = CollectIntValue(ss, "/track/count", this->trackCount, CountTracks(project), dump);

	const int LENGTH = 20;
	char name[LENGTH];

	for (int index = 0; index < this->trackBankSize; index++)
	{
		std::stringstream das;
		das << "/track/" << bankTrackIndex << "/";
		std::string trackAddress = das.str();

		// Track exists flag and number of tracks
		CollectIntArrayValue(ss, (trackAddress + "exists").c_str(), index, this->trackExists, trackIndex < trackCount ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "number").c_str(), index, this->trackNumber, trackIndex, dump);

		// Track name
		MediaTrack *track = GetTrack(project, trackIndex);
		bool result = track != nullptr && GetTrackName(track, name, LENGTH);
		CollectStringArrayValue(ss, (trackAddress + "name").c_str(), index, this->trackName, result ? name : "", dump);

		// Track type (GROUP or HYBRID), select, mute, solo, recarm and monitor states
		int trackState;
		if (track != nullptr)
			GetTrackState(track, &trackState);
		CollectStringArrayValue(ss, (trackAddress + "type").c_str(), index, this->trackType, (trackState & 1) > 0 ? "GROUP" : "HYBRID", dump);
		int selected = (trackState & 2) > 0 ? 1 : 0;
		if (trackIndex < trackCount && selected)
			this->trackSelection = index;

		CollectIntArrayValue(ss, (trackAddress + "select").c_str(), index, this->trackSelected, selected, dump);
		CollectIntArrayValue(ss, (trackAddress + "mute").c_str(), index, this->trackMute, (trackState & 8) > 0 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "solo").c_str(), index, this->trackSolo, (trackState & 16) > 0 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "recarm").c_str(), index, this->trackRecArmed, (trackState & 64) > 0 ? 1 : 0, dump);
		// Uses "lock track" as active indication
		CollectIntArrayValue(ss, (trackAddress + "active").c_str(), index, this->trackActive, track != nullptr && GetTrackLockState(track) ? 0 : 1, dump);

		double monitor = track != nullptr ? GetMediaTrackInfo_Value(track, "I_RECMON") : 0;
		CollectIntArrayValue(ss, (trackAddress + "monitor").c_str(), index, this->trackMonitor, monitor == 1 ? 1 : 0, dump);
		CollectIntArrayValue(ss, (trackAddress + "autoMonitor").c_str(), index, this->trackAutoMonitor, monitor == 2 ? 1 : 0, dump);

		// Track color
		int red = 0, green = 0, blue = 0;
		if (track != nullptr)
			ColorFromNative(GetTrackColor(track) & 0xFEFFFFFF, &red, &green, &blue);
		CollectStringArrayValue(ss, (trackAddress + "color").c_str(), index, this->trackColor, FormatColor(red, green, blue).c_str(), dump);

		// Track volume and pan
		double volDB = track != nullptr ? this->model.ValueToDB(GetMediaTrackInfo_Value(track, "D_VOL")) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "volume").c_str(), index, this->trackVolume, DB2SLIDER(volDB) / 1000.0, dump);
		CollectStringArrayValue(ss, (trackAddress + "volume/str").c_str(), index, this->trackVolumeStr, FormatDB(volDB).c_str(), dump);
		double panVal = track != nullptr ? GetMediaTrackInfo_Value(track, "D_PAN") : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "pan").c_str(), index, this->trackPan, (panVal + 1) / 2, dump);
		CollectStringArrayValue(ss, (trackAddress + "pan/str").c_str(), index, this->trackPanStr, FormatPan(panVal).c_str(), dump);

		// VU and automation mode
		double peak = track != nullptr ? Track_GetPeakInfo(track, 0) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "vuleft").c_str(), index, this->trackVULeft, DB2SLIDER(this->model.ValueToDB(peak)) / 1000.0, dump);
		peak = track != nullptr ? Track_GetPeakInfo(track, 1) : 0;
		CollectDoubleArrayValue(ss, (trackAddress + "vuright").c_str(), index, this->trackVURight, DB2SLIDER(this->model.ValueToDB(peak)) / 1000.0, dump);
		double automode = track != nullptr ? GetMediaTrackInfo_Value(track, "I_AUTOMODE") : 0;
		CollectIntArrayValue(ss, (trackAddress + "automode").c_str(), index, this->trackAutoMode, (int)automode, dump);

		// Sends
		int numSends = track != nullptr ? GetTrackNumSends(track, 0) : 0;
		for (int sendCounter = 0; sendCounter < this->sendBankSize; sendCounter++)
		{
			std::stringstream stream;
			stream << trackAddress << "send/" << sendCounter + 1 << "/";
			std::string sendAddress = stream.str();
			int arrayIndex = index * this->trackBankSize + sendCounter;
			if (sendCounter < numSends)
			{
				result = GetTrackSendName(track, sendCounter, name, LENGTH);
				CollectStringArrayValue(ss, (sendAddress + "name").c_str(), index, this->trackSendName[sendCounter], result ? name : "", dump);
				volDB = this->model.ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
				CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), index, this->trackSendVolume[sendCounter], DB2SLIDER(volDB) / 1000.0, dump);
				CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), index, this->trackSendVolumeStr[sendCounter], FormatDB(volDB).c_str(), dump);
			}
			else
			{
				CollectStringArrayValue(ss, (sendAddress + "name").c_str(), index, this->trackSendName[sendCounter], "", dump);
				CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), index, this->trackSendVolume[sendCounter], 0, dump);
				CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), index, this->trackSendVolumeStr[sendCounter], "", dump);
			}
		}

		// Midi note repeat plugin is on track?
		int position = track != nullptr ? TrackFX_AddByName(track, "midi_note_repeater", 1, 0) : -1;
		int repeatActive = position > -1 && TrackFX_GetEnabled(track, 0x1000000 + position) ? 1 : 0;
		double minVal, maxVal;
		int repeatNoteLength = position > -1 ? (int)TrackFX_GetParam(track, 0x1000000 + position, 0, &minVal, &maxVal) : 1;
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
	double volDB = this->model.ValueToDB(GetMediaTrackInfo_Value(master, "D_VOL"));
	this->model.masterVolume = CollectDoubleValue(ss, "/master/volume", this->model.masterVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->masterVolumeStr = CollectStringValue(ss, "/master/volume/str", this->masterVolumeStr, FormatDB(volDB).c_str(), dump);

	double panVal = GetMediaTrackInfo_Value(master, "D_PAN");
	this->model.masterPan = CollectDoubleValue(ss, "/master/pan", this->model.masterPan, (panVal + 1) / 2, dump);
	this->masterPanStr = CollectStringValue(ss, "/master/pan/str", this->masterPanStr, FormatPan(panVal).c_str(), dump);

	this->masterVULeft = CollectDoubleValue(ss, "/master/vuleft", this->masterVULeft, DB2SLIDER(this->model.ValueToDB(Track_GetPeakInfo(master, 0))) / 1000.0, dump);
	this->masterVURight = CollectDoubleValue(ss, "/master/vuright", this->masterVURight, DB2SLIDER(this->model.ValueToDB(Track_GetPeakInfo(master, 1))) / 1000.0, dump);
}

/**
 * Collect the (changed) groove data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectGrooveData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	if (!APIExists("BR_Win32_GetPrivateProfileString"))
		return;

	const char *configFile = get_ini_file();
	if (configFile == nullptr)
		return;

	// TODO All SWS functions crash?
	//const int LENGTH = 20;
	//char result[LENGTH];
	//BR_Win32_GetPrivateProfileString("fingers", "groove_strength", "100", configFile, result, LENGTH);
	//this->grooveStrength = CollectIntValue(ss, "/groove/strength", this->grooveStrength, std::atoi(result), dump);

	//BR_Win32_GetPrivateProfileString("fingers", "groove_velstrength", "100", configFile, result, LENGTH);
	//this->grooveVelstrength = CollectIntValue(ss, "/groove/velocity", this->grooveVelstrength, std::atoi(result), dump);

	//BR_Win32_GetPrivateProfileString("fingers", "groove_target", "0", configFile, result, LENGTH);
	//this->grooveTarget = CollectIntValue(ss, "/groove/target", this->grooveTarget, std::atoi(result), dump);

	//BR_Win32_GetPrivateProfileString("fingers", "groove_tolerance", "16", configFile, result, LENGTH);
	//this->grooveTolerance = CollectIntValue(ss, "/groove/tolerance", this->grooveTolerance, std::atoi(result), dump);

	//BR_Win32_GetPrivateProfileString("midiedit", "quantstrength", "100", configFile, result, LENGTH);
	//this->quantizeStrength = CollectIntValue(ss, "/quantize/strength", this->quantizeStrength, std::atoi(result), dump);
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
	double musicalStart = -1;
	double musicalEnd = -1;
	int timesig;
	int denomOut;
	double startBPM;
	double endBPM;
	int count = CountSelectedMediaItems(project);
	if (count > 0)
	{
		MediaItem *item = GetSelectedMediaItem(project, 0);
		double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
		TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &endBPM);
		double musicalStart = startBPM * itemStart / 60;
		double musicalEnd = endBPM * itemEnd / 60;
	}
	this->clipMusicalStart = CollectDoubleValue(ss, "/clip/start", this->clipMusicalStart, musicalStart, dump);
	this->clipMusicalEnd = CollectDoubleValue(ss, "/clip/end", this->clipMusicalEnd, musicalEnd, dump);

	// Get the loop start and end if any
	double loopStart;
	double loopEnd;
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


void DataCollector::AdjustTrackBank(ReaProject *project)
{
	MediaTrack *track = GetSelectedTrack(project, 0);
	if (track == nullptr)
		return;
	int trackIdx = CSurf_TrackToID(track, false) - 1;
	if (trackIdx >= 0)
		this->trackBankOffset = (int)std::floor(trackIdx / this->trackBankSize) * this->trackBankSize;
}


int DataCollector::GetTrackLockState(MediaTrack *track)
{
	char xmlStr[10000];
	if (!GetTrackStateChunk(track, xmlStr, 10000, false))
		return 0;
	std::cmatch result;
	if (!std::regex_search(xmlStr, result, trackLockPattern))
		return 0;
	std::string value = result.str(1);
	return std::atoi(value.c_str());
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
		ss << command << " " << newValue << "\n";
		currentValues.at(index).assign(newValue);
	}
}


void DataCollector::CollectDoubleArrayValue(std::stringstream &ss, const char *command, int index, std::vector<double> &currentValues, double newValue, const bool &dump) const
{
	if (std::fabs(currentValues.at(index) - newValue) > 0.0000000001 || dump)
	{
		ss << command << " " << newValue << "\n";
		currentValues[index] = newValue;
	}
}


void DataCollector::CollectIntArrayValue(std::stringstream &ss, const char *command, int index, std::vector<int> &currentValues, int newValue, const bool &dump) const
{
	if (currentValues.at(index) != newValue || dump)
	{
		ss << command << " " << newValue << "\n";
		currentValues[index] = newValue;
	}
}


std::string DataCollector::FormatColor(int red, int green, int blue)
{
	std::stringstream ss;
	ss << red << " " << green << " " << blue;
	return ss.str();
}


std::string DataCollector::FormatDB(double value)
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


std::string DataCollector::FormatPan(double value)
{
	if (abs(value) < 0.001)
		return "C";
	std::stringstream stream;
	if (value < 0)
		stream << (int)(value * -100) << "L";
	else
		stream << (int)(value * 100) << "R";
	return stream.str();
}
