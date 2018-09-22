// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <fstream>

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
	for (int index = 0; index < paramCount; index++)
	{
		Parameter *parameter = this->model.GetParameter(index);
		parameter->CollectData(ss, track, deviceIndex, index, paramCount, dump);
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
	int count = CountTracks(project);
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
		Track *track = this->model.GetTrack(trackIndex);
		track->CollectData(ss, project, mediaTrack, trackIndex, dump);
		trackIndex++;
	}
	this->model.trackCount = Collectors::CollectIntValue(ss, "/track/count", this->model.trackCount, trackIndex, dump);
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

		int clipColor = (int)GetDisplayedMediaItemColor(item);
		ColorFromNative(clipColor & 0xFEFFFFFF, &red, &green, &blue);
	}
	std::string newNotes = this->CollectClipNotes(project, item);
	this->notesStr = Collectors::CollectStringValue(ss, "/clip/notes", this->notesStr, newNotes.c_str(), dump);

	this->clipMusicalStart = Collectors::CollectDoubleValue(ss, "/clip/start", this->clipMusicalStart, musicalStart, dump);
	this->clipMusicalEnd = Collectors::CollectDoubleValue(ss, "/clip/end", this->clipMusicalEnd, musicalEnd, dump);
	this->clipMusicalPlayPosition = Collectors::CollectDoubleValue(ss, "/clip/playposition", this->clipMusicalPlayPosition, musicalPlayPosition, dump);

	this->clipColor = Collectors::CollectStringValue(ss, "/clip/color", this->clipColor, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


/**
 * Collect the notes data of the active clip if changed.
 *
 * @param project The current Reaper project
 * @param item The media item
 * @return The formatted collected notes
 */
std::string DataCollector::CollectClipNotes(ReaProject *project, MediaItem *item)
{
	std::string notesStrNew{ " " };

	if (item == nullptr)
	{
		this->noteHash = "";
		return notesStrNew;
	}
	MediaItem_Take *take = GetActiveTake(item);
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

	int pitch, velocity;
	double startppqpos{ -1 }, endppqpos{ -1 }, musicalStart{ -1 }, musicalEnd{ -1 };
	double bpm{};
	std::stringstream notes;
	double pos = GetMediaItemInfo_Value(item, "D_POSITION");

	for (int i = 0; i < noteCount; ++i)
	{
		MIDI_GetNote(take, i, nullptr, nullptr, &startppqpos, &endppqpos, nullptr, &pitch, &velocity);

		musicalStart = MIDI_GetProjTimeFromPPQPos(take, startppqpos);
		musicalEnd = MIDI_GetProjTimeFromPPQPos(take, endppqpos);
		musicalStart -= pos;
		musicalEnd -= pos;

		TimeMap_GetTimeSigAtTime(project, startppqpos, nullptr, nullptr, &bpm);
		musicalStart = bpm * musicalStart / 60.0;
		TimeMap_GetTimeSigAtTime(project, endppqpos, nullptr, nullptr, &bpm);
		musicalEnd = bpm * musicalEnd / 60.0;
		notes << musicalStart << ":" << musicalEnd << ":" << pitch << ":" << velocity << ";";
	}
	return notes.str().c_str();
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
	const std::vector<int> markers = Marker::GetMarkers(project);
	const int count = (int) markers.size();
	this->model.markerCount = Collectors::CollectIntValue(ss, "/marker/count", this->model.markerCount, count, dump);
	for (int index = 0; index < count; index++)
	{
		Marker *marker = this->model.GetMarker(markers.at(index));
		marker->CollectData(ss, project, "marker", index, markers.at(index), dump);
	}
}


/**
 * Collect the (changed) clip data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param dump If true all data is collected not only the changed one since the last call
 */
void DataCollector::CollectSessionData(std::stringstream &ss, ReaProject *project, const bool &dump)
{
	// Only collect clip data if document has changed
	const int state = GetProjectStateChangeCount(project);
	if (this->projectState == state)
		return;
	this->projectState = state;

	std::stringstream clipstr;

	int count = CountTracks(project);
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

		int red = 0, green = 0, blue = 0;
		char buf[2048];

		const int itemCount = CountTrackMediaItems(mediaTrack);

		clipstr << trackIndex << ";" << itemCount << ";";

		for (int i = 0; i < itemCount; i++)
		{
			MediaItem *item = GetTrackMediaItem(mediaTrack, i);
			if (item == nullptr)
				continue;
			MediaItem_Take *take = GetActiveTake(item);
			if (take == nullptr)
				continue;

			// Format track index, clip index, name, selected state and color in one string

			if (GetSetMediaItemTakeInfo_String(take, "P_NAME", buf, false))
			{
				std::string s = buf;
				std::replace(s.begin(), s.end(), ';', ' ');
				clipstr << s;
			}

			const bool isSelected = IsMediaItemSelected(item);
			clipstr << ";" << isSelected;

			int clipColor = (int)GetDisplayedMediaItemColor(item);
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
		Marker *scene = this->model.GetRegion(regions.at(index));
		scene->CollectData(ss, project, "scene", index, regions.at(index), dump);
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
