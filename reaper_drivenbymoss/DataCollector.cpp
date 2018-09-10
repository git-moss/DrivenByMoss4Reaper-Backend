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
#include "Collectors.h"
#include "ReaperUtils.h"
#include "ReaDebug.h"


/**
 * Constructor.
 *
 * @param aModel The model
 */
DataCollector::DataCollector(Model &aModel) :
	model(aModel),
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
std::string DataCollector::CollectData(const bool &dump)
{
	std::stringstream ss;
	ReaProject *project = ReaperUtils::GetProject();
	
	CollectTransportData(ss, project, dump);
	CollectProjectData(ss, project, dump);
	CollectTrackData(ss, project, dump);
	CollectDeviceData(ss, project, dump);
	CollectMasterTrackData(ss, project, dump);
	CollectBrowserData(ss, project, dump);
	CollectMarkerData(ss, project, dump);
	CollectClipData(ss, project, dump);
	CollectSessionData(ss, project, dump);

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
	this->projectName = Collectors::CollectStringValue(ss, "/project/name", projectName, newProjectName, dump);
	this->projectEngine = Collectors::CollectIntValue(ss, "/project/engine", projectEngine, Audio_IsRunning(), dump);
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
	this->play = Collectors::CollectIntValue(ss, "/play", this->play, (playState & 1) > 0, dump);
	this->record = Collectors::CollectIntValue(ss, "/record", this->record, (playState & 4) > 0, dump);
	this->repeat = Collectors::CollectIntValue(ss, "/repeat", this->repeat, GetSetRepeat(-1), dump);
	this->metronome = Collectors::CollectIntValue(ss, "/click", this->metronome, GetToggleCommandState(40364), dump);

	// The tempo
	this->tempo = Collectors::CollectDoubleValue(ss, "/tempo", this->tempo, Master_GetTempo(), dump);

	// Get the time signature at the current play position
	double cursorPos = GetCursorPositionEx(project);
	int timesig;
	int denomOut;
	double startBPM;
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->globalTimesig = Collectors::CollectIntValue(ss, "/numerator", this->globalTimesig, timesig, dump);
	this->globalDenomOut = Collectors::CollectIntValue(ss, "/denominator", this->globalDenomOut, denomOut, dump);

	// Result is in seconds
	cursorPos = this->play ? GetPlayPositionEx(project) : GetCursorPositionEx(project);
	TimeMap_GetTimeSigAtTime(project, cursorPos, &timesig, &denomOut, &startBPM);
	this->playPosition = Collectors::CollectDoubleValue(ss, "/time", this->playPosition, cursorPos, dump);
	char timeStr[20];
	format_timestr(cursorPos, timeStr, 20);
	this->strPlayPosition = Collectors::CollectStringValue(ss, "/time/str", this->strPlayPosition, timeStr, dump);
	// 2 = measures.beats
	format_timestr_pos(cursorPos, timeStr, 20, 2);
	this->strBeatPosition = Collectors::CollectStringValue(ss, "/beat", this->strBeatPosition, timeStr, dump);

	this->prerollClick = Collectors::CollectIntValue(ss, "/prerollClick", this->prerollClick, GetToggleCommandState(41819), dump);
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
	MediaTrack *track = GetSelectedTrack(project, 0);

	const int deviceIndex = this->model.deviceBankOffset + this->model.deviceSelected;
	int bankDeviceIndex = 1;
	this->model.deviceCount = Collectors::CollectIntValue(ss, "/device/count", this->model.deviceCount, TrackFX_GetCount(track), dump);
	this->deviceExists = Collectors::CollectIntValue(ss, "/device/exists", this->deviceExists, deviceIndex < this->model.deviceCount ? 1 : 0, dump);
	this->devicePosition = Collectors::CollectIntValue(ss, "/device/position", this->devicePosition, deviceIndex, dump);
	this->deviceWindow = Collectors::CollectIntValue(ss, "/device/window", this->deviceWindow, TrackFX_GetOpen(track, deviceIndex), dump);
	this->deviceExpanded = Collectors::CollectIntValue(ss, "/device/expand", this->deviceExpanded, this->model.deviceExpandedType == 1, dump);

	const int LENGTH = 20;
	char name[LENGTH];
	bool result = TrackFX_GetFXName(track, deviceIndex, name, LENGTH);
	this->deviceName = Collectors::CollectStringValue(ss, "/device/name", this->deviceName, result ? name : "", dump);
	this->deviceBypass = Collectors::CollectIntValue(ss, "/device/bypass", this->deviceBypass, TrackFX_GetEnabled(track, deviceIndex) ? 0 : 1, dump);

	for (int index = 0; index < this->model.deviceBankSize; index++)
	{
		std::stringstream das;
		das << "/device/sibling/" << bankDeviceIndex << "/name";
		std::string deviceAddress = das.str();
		result = TrackFX_GetFXName(track, this->model.deviceBankOffset + index, name, LENGTH);
		Collectors::CollectStringArrayValue(ss, das.str().c_str(), index, deviceSiblings, result ? name : "", dump);
		bankDeviceIndex++;
	}

	const int paramCount = TrackFX_GetNumParams(track, deviceIndex);
	this->model.deviceParamCount = Collectors::CollectIntValue(ss, "/device/param/count", this->model.deviceParamCount, paramCount, dump);
	this->model.deviceParamBankSelected = Collectors::CollectIntValue(ss, "/device/param/bank/selected", this->model.deviceParamBankSelected, this->model.deviceParamBankSelectedTemp, dump);

	int paramIndex = this->model.deviceParamBankSelected * this->model.parameterBankSize;
	for (int index = 0; index < this->model.parameterBankSize; index++)
	{
		std::stringstream das;
		das << "/device/param/" << index + 1 << "/";
		std::string paramAddress = das.str();

		result = TrackFX_GetParamName(track, deviceIndex, paramIndex, name, LENGTH);
		Collectors::CollectStringArrayValue(ss, (paramAddress + "name").c_str(), index, this->deviceParamName, result ? name : "", dump);
		const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, paramIndex);
		Collectors::CollectDoubleArrayValue(ss, (paramAddress + "value").c_str(), index, this->deviceParamValue, paramValue, dump);
		result = TrackFX_FormatParamValueNormalized(track, deviceIndex, paramIndex, paramValue, name, LENGTH);
		Collectors::CollectStringArrayValue(ss, (paramAddress + "value/str").c_str(), index, this->deviceParamValueStr, result ? name : "", dump);

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
	this->model.trackCount = Collectors::CollectIntValue(ss, "/track/count", this->model.trackCount, CountTracks(project), dump);
	int count = this->model.trackCount;
	for (int index = 0; index < count; index++)
	{
		Track *track = this->model.GetTrack(index);
		track->CollectData(ss, project, index, count, dump);
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

	Collectors::CollectIntValue(ss, "/master/select", this->masterSelected, (trackState & 2) > 0, dump);
	Collectors::CollectIntValue(ss, "/master/mute", this->masterMute, (trackState & 8) > 0 ? 1 : 0, dump);
	Collectors::CollectIntValue(ss, "/master/solo", this->masterSolo, (trackState & 16) > 0 ? 1 : 0, dump);

	// Master track volume and pan
	const double volDB = ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(master, "D_VOL"));
	this->model.masterVolume = Collectors::CollectDoubleValue(ss, "/master/volume", this->model.masterVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->masterVolumeStr = Collectors::CollectStringValue(ss, "/master/volume/str", this->masterVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);

	const double panVal = GetMediaTrackInfo_Value(master, "D_PAN");
	this->model.masterPan = Collectors::CollectDoubleValue(ss, "/master/pan", this->model.masterPan, (panVal + 1) / 2, dump);
	this->masterPanStr = Collectors::CollectStringValue(ss, "/master/pan/str", this->masterPanStr, Collectors::FormatPan(panVal).c_str(), dump);

	this->masterVULeft = Collectors::CollectDoubleValue(ss, "/master/vuleft", this->masterVULeft, DB2SLIDER(ReaperUtils::ValueToDB(Track_GetPeakInfo(master, 0))) / 1000.0, dump);
	this->masterVURight = Collectors::CollectDoubleValue(ss, "/master/vuright", this->masterVURight, DB2SLIDER(ReaperUtils::ValueToDB(Track_GetPeakInfo(master, 1))) / 1000.0, dump);
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
	double musicalPlayPosition{ -1 };
	double bpm{};
	const int count = CountSelectedMediaItems(project);
	int red = 0, green = 0, blue = 0;
	MediaItem *item = nullptr;
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

		// TODO Test - int GetDisplayedMediaItemColor2(MediaItem* item, MediaItem_Take* take)
		int clipColor = (int)GetDisplayedMediaItemColor(item);
		ColorFromNative(clipColor & 0xFEFFFFFF, &red, &green, &blue);
	}
	this->CollectClipNotes(ss, project, item, dump);

	this->clipMusicalStart = Collectors::CollectDoubleValue(ss, "/clip/start", this->clipMusicalStart, musicalStart, dump);
	this->clipMusicalEnd = Collectors::CollectDoubleValue(ss, "/clip/end", this->clipMusicalEnd, musicalEnd, dump);
	this->clipMusicalPlayPosition = Collectors::CollectDoubleValue(ss, "/clip/playposition", this->clipMusicalPlayPosition, musicalPlayPosition, dump);

	this->clipColor = Collectors::CollectStringValue(ss, "/clip/color", this->clipColor, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


/**
 * Collect the notes data of the active clip if changed.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param item The media item
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectClipNotes(std::stringstream &ss, ReaProject *project, MediaItem *item, const bool &dump)
{
	std::string notesStrNew{};

	if (item)
	{
		MediaItem_Take *take = GetActiveTake(item);
		if (take)
		{
			char nhash[16];
			if (!MIDI_GetHash(take, true, nhash, 16))
				return;

			if (this->noteHash.compare(nhash) == 0)
				return;

			this->noteHash = nhash;

			notesStrNew = " ";

			int noteCount;
			if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr))
			{
				int pitch, velocity;
				double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };
				double bpm{};
				std::stringstream notes;
				double pos = GetMediaItemInfo_Value(item, "D_POSITION");

				if (noteCount > 0)
				{
					for (int i = 0; i < noteCount; ++i)
					{
						MIDI_GetNote(take, i, nullptr, nullptr, &startppqpos, &endppqpos, nullptr, &pitch, &velocity);

						musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
						musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);

						// TODO -pos is not (fully) correct but output looks good?!
						// ReaDebug() << "Time (s): Pos: " << pos << " : Start: " << musicalStart << " - " << musicalEnd;
						musicalStart -= pos;
						musicalEnd -= pos;
						// ReaDebug() << "Time (s) moved: " << musicalStart << " - " << musicalEnd;

						TimeMap_GetTimeSigAtTime(project, startppqpos, nullptr, nullptr, &bpm);
						musicalStart = bpm * musicalStart / 60.0;
						TimeMap_GetTimeSigAtTime(project, endppqpos, nullptr, nullptr, &bpm);
						musicalEnd = bpm * musicalEnd / 60.0;
						notes << musicalStart << ":" << musicalEnd << ":" << pitch << ":" << velocity << ";";

						// ReaDebug() << "Musical (qn): " << musicalStart;

					}
					notesStrNew = notes.str().c_str();
				}
			}
		}
		else
			this->noteHash = "";
	}
	else
		this->noteHash = "";

	this->notesStr = Collectors::CollectStringValue(ss, "/clip/notes", this->notesStr, notesStrNew.c_str(), dump);
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
	MediaTrack *track = GetSelectedTrack(project, 0);
	const int sel = this->model.deviceBankOffset + this->model.deviceSelected;
	LoadDevicePresetFile(ss, track, sel, dump);

	const int LENGTH = 20;
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
void DataCollector::CollectMarkerData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	int markerIndex = this->model.markerBankOffset;
	int bankMarkerIndex = 1;

	this->model.markerCount = Collectors::CollectIntValue(ss, "/marker/count", this->model.markerCount, CountProjectMarkers(project, nullptr, nullptr), dump);

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
		Collectors::CollectIntArrayValue(ss, (markerAddress + "exists").c_str(), index, this->markerExists, exists, dump);
		Collectors::CollectIntArrayValue(ss, (markerAddress + "number").c_str(), index, this->markerNumber, markerIndex, dump);

		int result = exists ? EnumProjectMarkers3(project, index, &isRegion, &markerPos, &regionEnd, &name, &markerRegionIndexNumber, &markerColor) : 0;

		// Marker name
		Collectors::CollectStringArrayValue(ss, (markerAddress + "name").c_str(), index, this->markerName, result ? name : "", dump);

		// Marker color
		int red = 0, green = 0, blue = 0;
		if (exists)
			ColorFromNative(markerColor & 0xFEFFFFFF, &red, &green, &blue);
		Collectors::CollectStringArrayValue(ss, (markerAddress + "color").c_str(), index, this->markerColor, Collectors::FormatColor(red, green, blue).c_str(), dump);

		markerIndex += 1;
		bankMarkerIndex += 1;
	}
}


void DataCollector::CollectSessionData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	for (int t = 0; t < CountTracks(project); t++)
	{
		MediaTrack *track = GetTrack(project, t);

		for (int i = 0; i < CountTrackMediaItems(track); i++)
		{
			MediaItem *item = GetTrackMediaItem(track, i);
			MediaItem_Take *take = GetActiveTake(item);
			if (take == nullptr)
				continue;

			char buf[2048];
			GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false);

			// TODO Implement

			/// track / {1 - 8} / clip / {1 - N} / name
			/// track / {1 - 8} / clip / {1 - N} / isSelected{ 1,0 }
			/// track / {1 - 8} / clip / {1 - N} / color             with rgb(r, g, b).r, g, b = 0..255

		}
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
				Collectors::CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, strip.c_str(), dump);

				counter += 1;
			}
		}
		file.close();

		while (counter < 128)
		{
			std::stringstream das;
			das << "/browser/result/" << counter + 1 << "/name";
			Collectors::CollectStringArrayValue(ss, das.str().c_str(), counter, this->devicePresetsStr, "", dump);
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
