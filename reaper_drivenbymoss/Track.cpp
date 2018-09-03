// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <iostream>
#include <sstream>

#include "reaper_plugin_functions.h"
#include "Collectors.h"
#include "ReaperUtils.h"
#include "Track.h"


/**
 * Constructor.
 *
 * @param sendBankSize The number of sends
 */
Track::Track(const int numSends) :
	trackSendName(numSends, ""),
	trackSendVolume(numSends, 0),
	trackSendVolumeStr(numSends, ""),
	sendBankSize(numSends)
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
 * @param trackIndex The index of the track
 * @param bankTrackIndex The index in the bank
 * @param trackCount The number of all tracks
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Track::CollectData(std::stringstream &ss, ReaProject *project, int trackIndex, int bankTrackIndex, int trackCount, const bool &dump)
{
	const int LENGTH = 20;
	char name[LENGTH];

	std::stringstream das;
	das << "/track/" << bankTrackIndex << "/";
	std::string trackAddress = das.str();

	// Track exists flag and number of tracks
	this->trackExists = Collectors::CollectIntValue(ss, (trackAddress + "exists").c_str(), this->trackExists, trackIndex < trackCount ? 1 : 0, dump);
	this->trackNumber = Collectors::CollectIntValue(ss, (trackAddress + "number").c_str(), this->trackNumber, trackIndex, dump);

	// Track name
	MediaTrack *track = GetTrack(project, trackIndex);
	bool result = track != nullptr && GetTrackName(track, name, LENGTH);
	this->trackName = Collectors::CollectStringValue(ss, (trackAddress + "name").c_str(), this->trackName, result ? name : "", dump);

	// Track type (GROUP or HYBRID), select, mute, solo, recarm and monitor states
	int trackState{};
	if (track != nullptr)
		GetTrackState(track, &trackState);
	this->trackType = Collectors::CollectStringValue(ss, (trackAddress + "type").c_str(), this->trackType, (trackState & 1) > 0 ? "GROUP" : "HYBRID", dump);
	const int selected = (trackState & 2) > 0 ? 1 : 0;
	this->trackSelected = Collectors::CollectIntValue(ss, (trackAddress + "select").c_str(), this->trackSelected, selected, dump);
	this->trackMute = Collectors::CollectIntValue(ss, (trackAddress + "mute").c_str(), this->trackMute, (trackState & 8) > 0 ? 1 : 0, dump);
	this->trackSolo = Collectors::CollectIntValue(ss, (trackAddress + "solo").c_str(), this->trackSolo, (trackState & 16) > 0 ? 1 : 0, dump);
	this->trackRecArmed = Collectors::CollectIntValue(ss, (trackAddress + "recarm").c_str(), this->trackRecArmed, (trackState & 64) > 0 ? 1 : 0, dump);
	// Uses "lock track" as active indication
	this->trackActive = Collectors::CollectIntValue(ss, (trackAddress + "active").c_str(), this->trackActive, track != nullptr && GetTrackLockState(track) ? 0 : 1, dump);

	const double monitor = track != nullptr ? GetMediaTrackInfo_Value(track, "I_RECMON") : 0;
	this->trackMonitor = Collectors::CollectIntValue(ss, (trackAddress + "monitor").c_str(), this->trackMonitor, monitor == 1 ? 1 : 0, dump);
	this->trackAutoMonitor = Collectors::CollectIntValue(ss, (trackAddress + "autoMonitor").c_str(), this->trackAutoMonitor, monitor == 2 ? 1 : 0, dump);

	// Track color
	int red = 0, green = 0, blue = 0;
	if (track != nullptr)
		ColorFromNative(GetTrackColor(track) & 0xFEFFFFFF, &red, &green, &blue);
	this->trackColor = Collectors::CollectStringValue(ss, (trackAddress + "color").c_str(), this->trackColor, Collectors::FormatColor(red, green, blue).c_str(), dump);

	// Track volume and pan
	double volDB = track != nullptr ? ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(track, "D_VOL")) : 0;
	this->trackVolume = Collectors::CollectDoubleValue(ss, (trackAddress + "volume").c_str(), this->trackVolume, DB2SLIDER(volDB) / 1000.0, dump);
	this->trackVolumeStr = Collectors::CollectStringValue(ss, (trackAddress + "volume/str").c_str(), this->trackVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);
	const double panVal = track != nullptr ? GetMediaTrackInfo_Value(track, "D_PAN") : 0;
	this->trackPan = Collectors::CollectDoubleValue(ss, (trackAddress + "pan").c_str(), this->trackPan, (panVal + 1) / 2, dump);
	this->trackPanStr = Collectors::CollectStringValue(ss, (trackAddress + "pan/str").c_str(), this->trackPanStr, Collectors::FormatPan(panVal).c_str(), dump);

	// VU and automation mode
	double peak = track != nullptr ? Track_GetPeakInfo(track, 0) : 0;
	this->trackVULeft = Collectors::CollectDoubleValue(ss, (trackAddress + "vuleft").c_str(), this->trackVULeft, DB2SLIDER(ReaperUtils::ValueToDB(peak)) / 1000.0, dump);
	peak = track != nullptr ? Track_GetPeakInfo(track, 1) : 0;
	this->trackVURight = Collectors::CollectDoubleValue(ss, (trackAddress + "vuright").c_str(), this->trackVURight, DB2SLIDER(ReaperUtils::ValueToDB(peak)) / 1000.0, dump);
	const double automode = track != nullptr ? GetMediaTrackInfo_Value(track, "I_AUTOMODE") : 0;
	this->trackAutoMode = Collectors::CollectIntValue(ss, (trackAddress + "automode").c_str(), this->trackAutoMode, static_cast<int>(automode), dump);

	// Sends
	const int numSends = track != nullptr ? GetTrackNumSends(track, 0) : 0;
	for (int sendCounter = 0; sendCounter < this->sendBankSize; sendCounter++)
	{
		std::stringstream stream;
		stream << trackAddress << "send/" << sendCounter + 1 << "/";
		std::string sendAddress = stream.str();
		if (sendCounter < numSends)
		{
			result = GetTrackSendName(track, sendCounter, name, LENGTH);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "name").c_str(), sendCounter, this->trackSendName, result ? name : "", dump);
			volDB = ReaperUtils::ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
			Collectors::CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), sendCounter, this->trackSendVolume, DB2SLIDER(volDB) / 1000.0, dump);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), sendCounter, this->trackSendVolumeStr, Collectors::FormatDB(volDB).c_str(), dump);
		}
		else
		{
			Collectors::CollectStringArrayValue(ss, (sendAddress + "name").c_str(), sendCounter, this->trackSendName, "", dump);
			Collectors::CollectDoubleArrayValue(ss, (sendAddress + "volume").c_str(), sendCounter, this->trackSendVolume, 0, dump);
			Collectors::CollectStringArrayValue(ss, (sendAddress + "volume/str").c_str(), sendCounter, this->trackSendVolumeStr, "", dump);
		}
	}

	// Midi note repeat plugin is on track?
	const int position = track != nullptr ? TrackFX_AddByName(track, "midi_note_repeater", 1, 0) : -1;
	const int repeatActive = position > -1 && TrackFX_GetEnabled(track, 0x1000000 + position) ? 1 : 0;
	double minVal{}, maxVal{};
	const int repeatNoteLength = position > -1 ? (int)TrackFX_GetParam(track, 0x1000000 + position, 0, &minVal, &maxVal) : 1;
	this->trackRepeatActive = Collectors::CollectIntValue(ss, (trackAddress + "repeatActive").c_str(), this->trackRepeatActive, repeatActive ? 1 : 0, dump);
	this->trackRepeatNoteLength = Collectors::CollectIntValue(ss, (trackAddress + "noterepeatlength").c_str(), this->trackRepeatNoteLength, repeatNoteLength, dump);
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
