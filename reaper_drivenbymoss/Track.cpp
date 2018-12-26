// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "Collectors.h"
#include "Track.h"


/**
 * Constructor.
 *
 * @param numSends The number of sends
 */
Track::Track(const int numSends) :
	sendBankSize(numSends),
	sendName(numSends, ""),
	sendVolume(numSends, 0),
	sendVolumeStr(numSends, "")
{
	// Intentionally empty
}


/**
 * Destructor.
 */
Track::~Track()
{
	// Intentionally empty
}


/**
 * Collect the (changed) track data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param track The track
 * @param trackIndex The index of the track
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Track::CollectData(std::stringstream &ss, ReaProject *project, MediaTrack *track, int trackIndex, const bool &dump)
{
	std::stringstream das;
	das << "/track/" << trackIndex << "/";
	std::string trackAddress = das.str();

	// Track exists flag and number of track
	this->exists = Collectors::CollectIntValue(ss, (trackAddress + "exists").c_str(), this->exists, 1, dump);
	this->number = Collectors::CollectIntValue(ss, (trackAddress + "number").c_str(), this->number, trackIndex, dump);
	this->depth = Collectors::CollectIntValue(ss, (trackAddress + "depth").c_str(), this->depth, GetTrackDepth(track), dump);

	// Track name
	const int LENGTH{ 20 };
	char name[LENGTH];
	bool result = GetTrackName(track, name, LENGTH);
	this->name = Collectors::CollectStringValue(ss, (trackAddress + "name").c_str(), this->name, result ? name : "", dump);

	// Track type (GROUP or HYBRID), select, mute, solo, recarm and monitor states
	int trackState{};
	GetTrackState(track, &trackState);
	this->type = Collectors::CollectStringValue(ss, (trackAddress + "type").c_str(), this->type, (trackState & 1) > 0 ? "GROUP" : "HYBRID", dump);
	const int selected = (trackState & 2) > 0 ? 1 : 0;
	this->isSelected = Collectors::CollectIntValue(ss, (trackAddress + "select").c_str(), this->isSelected, selected, dump);
	this->mute = Collectors::CollectIntValue(ss, (trackAddress + "mute").c_str(), this->mute, (trackState & 8) > 0 ? 1 : 0, dump);
	this->solo = Collectors::CollectIntValue(ss, (trackAddress + "solo").c_str(), this->solo, (trackState & 16) > 0 ? 1 : 0, dump);
	this->recArmed = Collectors::CollectIntValue(ss, (trackAddress + "recarm").c_str(), this->recArmed, (trackState & 64) > 0 ? 1 : 0, dump);
	// Uses "lock track" as active indication
	this->isActive = Collectors::CollectIntValue(ss, (trackAddress + "active").c_str(), this->isActive, GetTrackLockState(track) ? 0 : 1, dump);

	const double monitor = GetMediaTrackInfo_Value(track, "I_RECMON");
	this->monitor = Collectors::CollectIntValue(ss, (trackAddress + "monitor").c_str(), this->monitor, monitor == 1 ? 1 : 0, dump);
	this->autoMonitor = Collectors::CollectIntValue(ss, (trackAddress + "autoMonitor").c_str(), this->autoMonitor, monitor == 2 ? 1 : 0, dump);

	// Track color
	int red = -1, green = -1, blue = -1;
	int nativeColor = GetTrackColor(track);
	if (nativeColor != 0)
		ColorFromNative(nativeColor & 0xFEFFFFFF, &red, &green, &blue);
	this->color = Collectors::CollectStringValue(ss, (trackAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);

	// Track volume and pan
	double volDB = ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(track, "D_VOL"));
	this->volume = Collectors::CollectDoubleValue(ss, (trackAddress + "volume").c_str(), this->volume, DB2SLIDER(volDB) / 1000.0, dump);
	this->volumeStr = Collectors::CollectStringValue(ss, (trackAddress + "volume/str").c_str(), this->volumeStr, Collectors::FormatDB(volDB).c_str(), dump);
	const double panVal = GetMediaTrackInfo_Value(track, "D_PAN");
	this->pan = Collectors::CollectDoubleValue(ss, (trackAddress + "pan").c_str(), this->pan, (panVal + 1) / 2, dump);
	this->panStr = Collectors::CollectStringValue(ss, (trackAddress + "pan/str").c_str(), this->panStr, Collectors::FormatPan(panVal).c_str(), dump);

	// VU and automation mode
	double peak = Track_GetPeakInfo(track, 0);
	this->vuLeft = Collectors::CollectDoubleValue(ss, (trackAddress + "vuleft").c_str(), this->vuLeft, DB2SLIDER(ReaperUtils::ValueToDB(peak)) / 1000.0, dump);
	peak = Track_GetPeakInfo(track, 1);
	this->vuRight = Collectors::CollectDoubleValue(ss, (trackAddress + "vuright").c_str(), this->vuRight, DB2SLIDER(ReaperUtils::ValueToDB(peak)) / 1000.0, dump);
	const double automode = GetMediaTrackInfo_Value(track, "I_AUTOMODE");
	this->autoMode = Collectors::CollectIntValue(ss, (trackAddress + "automode").c_str(), this->autoMode, static_cast<int>(automode), dump);

	// Sends
	const int numSends = GetTrackNumSends(track, 0);
	char sendName[LENGTH];
	for (int sendCounter = 0; sendCounter < this->sendBankSize; sendCounter++)
	{
		std::stringstream stream;
		stream << trackAddress << "send/" << sendCounter + 1 << "/";
		std::string sendAddress = stream.str();
		if (sendCounter < numSends)
		{
			bool result = GetTrackSendName(track, sendCounter, sendName, LENGTH);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "name").c_str(), sendCounter, this->sendName, result ? sendName : "", dump);
			volDB = ReaperUtils::ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
			Collectors::CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), sendCounter, this->sendVolume, DB2SLIDER(volDB) / 1000.0, dump);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), sendCounter, this->sendVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);
		}
		else
		{
			Collectors::CollectStringArrayValue(ss, (sendAddress + "name").c_str(), sendCounter, this->sendName, "", dump);
			Collectors::CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), sendCounter, this->sendVolume, 0, dump);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), sendCounter, this->sendVolumeStr, "", dump);
		}
	}

	// Midi note repeat plugin is on track?
	const int position = TrackFX_AddByName(track, "midi_note_repeater", 1, 0);
	const int repeatActive = position > -1 && TrackFX_GetEnabled(track, 0x1000000 + position) ? 1 : 0;
	this->repeatActive = Collectors::CollectIntValue(ss, (trackAddress + "repeatActive").c_str(), this->repeatActive, repeatActive ? 1 : 0, dump);
	double minVal{}, maxVal{};
	const double repeatNoteLength = position > -1 ? TrackFX_GetParam(track, 0x1000000 + position, 0, &minVal, &maxVal) : 1.0;
	this->repeatNoteLength = Collectors::CollectDoubleValue(ss, (trackAddress + "noterepeatlength").c_str(), this->repeatNoteLength, repeatNoteLength, dump);
}


int Track::GetTrackLockState(MediaTrack *track)
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
