// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "Collectors.h"
#include "Send.h"


/**
 * Constructor.
 */
Send::Send()
{
	// Intentionally empty
}


/**
 * Destructor.
 */
Send::~Send()
{
	// Intentionally empty
}


/**
 * Collect the (changed) send data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param track The track
 * @param sendIndex The index of the send
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Send::CollectData(std::stringstream& ss, ReaProject* project, MediaTrack* track, int sendIndex, std::string& trackAddress, const bool& dump)
{
	std::stringstream stream;
	stream << trackAddress << "send/" << sendIndex << "/";
	std::string sendAddress = stream.str();
	const int LENGTH{ 20 };
	char name[LENGTH];
	bool result = GetTrackSendName(track, sendIndex, name, LENGTH);
	this->name = Collectors::CollectStringValue(ss, (sendAddress + "name").c_str(), this->name, result ? name : "", dump);
	double volDB = GetSendVolume(track, sendIndex, ReaperUtils::GetCursorPosition(project));
	this->volume = Collectors::CollectDoubleValue(ss, (sendAddress + "volume").c_str(), this->volume, DB2SLIDER(volDB) / 1000.0, dump);
	this->volumeStr = Collectors::CollectStringValue(ss, (sendAddress + "volume/str").c_str(), this->volumeStr, Collectors::FormatDB(volDB).c_str(), dump);
}


double Send::GetSendVolume(MediaTrack* track, int sendCounter, double position) const
{
	const char* sendType = "<VOLENV";
	TrackEnvelope* envelope = (TrackEnvelope*)GetSetTrackSendInfo(track, 0, sendCounter, "P_ENV", (void*)sendType);
	if (envelope != nullptr)
		return ReaperUtils::ValueToDB(ReaperUtils::GetEnvelopeValueAtPosition(envelope, position));
	return ReaperUtils::ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
}
