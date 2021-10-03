// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "WrapperGSL.h"
#include "Collectors.h"
#include "Track.h"

const std::regex Track::LOCK_PATTERN{ "LOCK\\s+(\\d+)" };
const std::regex Track::INPUT_QUANTIZE_PATTERN{ "INQ\\s+([0-9]+(\\.[0-9]+)?)\\s+(-?[0-9]+(\\.[0-9]+)?)\\s+([0-9]+(\\.[0-9]+)?)\\s+([0-9]+(\\.[0-9]+)?)\\s+" };


/**
 * Constructor.
 */
Track::Track() noexcept
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
 * @param slowUpdate If true, also update the data on the slow thread
 * @param readChunk If true, also read data only available via the track chunk (which is slow)
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Track::CollectData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, int trackIndex, const bool& slowUpdate, const bool& readChunk, const bool& dump)
{
	std::ostringstream das;
	das << "/track/" << trackIndex << "/";
	std::string trackAddress = das.str();

	double cursorPos = ReaperUtils::GetCursorPosition(project);

	// Track exists flag and number of track
	this->exists = Collectors::CollectIntValue(ss, (trackAddress + "exists").c_str(), this->exists, 1, dump);
	this->number = Collectors::CollectIntValue(ss, (trackAddress + "number").c_str(), this->number, trackIndex, dump);
	this->depth = Collectors::CollectIntValue(ss, (trackAddress + "depth").c_str(), this->depth, GetTrackDepth(track), dump);

	// Track name
	char tempName[NAME_LENGTH];
	bool result = GetTrackName(track, tempName, NAME_LENGTH);
	this->name = Collectors::CollectStringValue(ss, (trackAddress + "name").c_str(), this->name, result ? tempName : "", dump);

	// Track type (GROUP or HYBRID), select, mute, solo, recarm and monitor states
	int trackState{};
	GetTrackState(track, &trackState);
	this->type = Collectors::CollectStringValue(ss, (trackAddress + "type").c_str(), this->type, (trackState & 1) > 0 ? "GROUP" : "HYBRID", dump);
	const int selected = (trackState & 2) > 0 ? 1 : 0;
	this->isSelected = Collectors::CollectIntValue(ss, (trackAddress + "select").c_str(), this->isSelected, selected, dump);
	this->mute = Collectors::CollectIntValue(ss, (trackAddress + "mute").c_str(), this->mute, this->GetMute(track, cursorPos, trackState), dump);
	this->solo = Collectors::CollectIntValue(ss, (trackAddress + "solo").c_str(), this->solo, (trackState & 16) > 0 ? 1 : 0, dump);
	this->recArmed = Collectors::CollectIntValue(ss, (trackAddress + "recarm").c_str(), this->recArmed, (trackState & 64) > 0 ? 1 : 0, dump);

	if (readChunk)
	{
		// Only update the performance heavy chunk analysis when playback is stopped...
		if ((GetPlayStateEx(project) & 1) == 0)
		{
			// Attributes which need to be read from the track chunk...
			char tempChunk[CHUNK_LENGTH];
			if (slowUpdate && GetTrackStateChunk(track, tempChunk, CHUNK_LENGTH, false))
			{
				// Uses "lock track" as active indication
				this->isActive = Collectors::CollectIntValue(ss, (trackAddress + "active").c_str(), this->isActive, GetTrackLockState(tempChunk) ? 0 : 1, dump);

				this->ParseInputQuantize(ss, trackAddress, dump, tempChunk);
			}
		}
	}

	const double monitor = GetMediaTrackInfo_Value(track, "I_RECMON");
	this->monitor = Collectors::CollectIntValue(ss, (trackAddress + "monitor").c_str(), this->monitor, monitor == 1 ? 1 : 0, dump);
	this->autoMonitor = Collectors::CollectIntValue(ss, (trackAddress + "autoMonitor").c_str(), this->autoMonitor, monitor == 2 ? 1 : 0, dump);

	const double recMode = GetMediaTrackInfo_Value(track, "I_RECMODE");
	this->overdub = Collectors::CollectIntValue(ss, (trackAddress + "overdub").c_str(), this->overdub, recMode == 7 ? 1 : 0, dump);

	// Track color
	int red = -1, green = -1, blue = -1;
	int nativeColor = GetTrackColor(track);
	if (nativeColor != 0)
		ColorFromNative(nativeColor & 0xFEFFFFFF, &red, &green, &blue);
	this->color = Collectors::CollectStringValue(ss, (trackAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);

	// Track volume and pan
	double volDB = this->GetVolume(track, cursorPos);
	this->volume = Collectors::CollectDoubleValue(ss, (trackAddress + "volume").c_str(), this->volume, DB2SLIDER(volDB) / 1000.0, dump);
	this->volumeStr = Collectors::CollectStringValue(ss, (trackAddress + "volume/str").c_str(), this->volumeStr, Collectors::FormatDB(volDB).c_str(), dump);
	const double panVal = this->GetPan(track, cursorPos);
	this->pan = Collectors::CollectDoubleValue(ss, (trackAddress + "pan").c_str(), this->pan, (panVal + 1) / 2, dump);
	this->panStr = Collectors::CollectStringValue(ss, (trackAddress + "pan/str").c_str(), this->panStr, Collectors::FormatPan(panVal).c_str(), dump);

	// VU and automation mode
	double peakLeft = Track_GetPeakInfo(track, 0);
	double peakRight = Track_GetPeakInfo(track, 1);

	this->vu = Collectors::CollectDoubleValue(ss, (trackAddress + "vu").c_str(), this->vu, ReaperUtils::ValueToVURange((peakLeft + peakRight) / 2.0), dump);
	this->vuLeft = Collectors::CollectDoubleValue(ss, (trackAddress + "vuleft").c_str(), this->vuLeft, ReaperUtils::ValueToVURange(peakLeft), dump);
	this->vuRight = Collectors::CollectDoubleValue(ss, (trackAddress + "vuright").c_str(), this->vuRight, ReaperUtils::ValueToVURange(peakRight), dump);
	const double automode = GetMediaTrackInfo_Value(track, "I_AUTOMODE");
	this->autoMode = Collectors::CollectIntValue(ss, (trackAddress + "automode").c_str(), this->autoMode, static_cast<int>(automode), dump);

	// Sends
	const int numSends = GetTrackNumSends(track, 0);
	for (int sendCounter = 0; sendCounter < numSends; sendCounter++)
		this->GetSend(sendCounter)->CollectData(ss, project, track, sendCounter, trackAddress, dump);
	this->sendCount = Collectors::CollectIntValue(ss, (trackAddress + "send/count").c_str(), this->sendCount, numSends, dump);
}


/**
 * Get a send.
 *
 * @param index The index of the send.
 * @return The send, if none exists at the index a new instance is created automatically
 */
Send* Track::GetSend(const int index) noexcept
{
	const std::lock_guard<std::mutex> lock(this->sendlock);

	const int diff = index - gsl::narrow_cast<int>(this->sends.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->sends.push_back(new Send());
	}
	return this->sends.at(index);
}


double Track::GetVolume(MediaTrack* track, double position) const
{
	TrackEnvelope* envelope = GetTrackEnvelopeByName(track, "Volume");
	if (envelope == nullptr)
		return ReaperUtils::ValueToDB(GetMediaTrackInfo_Value(track, "D_VOL"));
	return ReaperUtils::ValueToDB(ReaperUtils::GetEnvelopeValueAtPosition(envelope, position));
}


double Track::GetPan(MediaTrack* track, double position) const
{
	TrackEnvelope* envelope = GetTrackEnvelopeByName(track, "Pan");
	if (envelope != nullptr)
	{
		// Higher values are left!
		return -1 * ReaperUtils::GetEnvelopeValueAtPosition(envelope, position);
	}
	return GetMediaTrackInfo_Value(track, "D_PAN");
}


int Track::GetMute(MediaTrack* track, double position, int trackState) const
{
	TrackEnvelope* envelope = GetTrackEnvelopeByName(track, "Mute");
	if (envelope != nullptr)
		return (int)ReaperUtils::GetEnvelopeValueAtPosition(envelope, position);
	return (trackState & 8) > 0 ? 1 : 0;
}


int Track::GetTrackLockState(char* chunk) const
{
	std::cmatch result{};
	if (!std::regex_search(chunk, result, LOCK_PATTERN))
		return 0;
	return std::atoi(result.str(1).c_str());
}


void Track::ParseInputQuantize(std::ostringstream& ss, std::string& trackAddress, const bool& dump, char* chunk)
{
	std::cmatch result{};
	if (!std::regex_search(chunk, result, INPUT_QUANTIZE_PATTERN))
		return;

	// "INQ 0 0 0 0.5 100 0 0 100" (Quantize Track Midi Recording (0/1), Positioning (-1/0/1), Quantize Note Offs (0/1), Period (double), ?, ?, ?, ?)
	int isEnabled = std::atoi(result.str(1).c_str());
	const int isLengthEnabled = std::atoi(result.str(5).c_str());
	double resolution = isEnabled == 0 ? 0 : std::atof(result.str(7).c_str());

	this->inQuantLengthEnabled = Collectors::CollectIntValue(ss, (trackAddress + "inQuantLengthEnabled").c_str(), this->inQuantLengthEnabled, isLengthEnabled, dump);
	this->inQuantResolution = Collectors::CollectDoubleValue(ss, (trackAddress + "inQuantResolution").c_str(), this->inQuantResolution, resolution, dump);
}