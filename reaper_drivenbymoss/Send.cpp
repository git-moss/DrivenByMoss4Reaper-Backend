// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "CodeAnalysis.h"
#include "Collectors.h"
#include "Send.h"


/**
 * Constructor.
 */
Send::Send() noexcept : enabled{ 0 }, volume{ 0 }
{
	// To make the MS analyzer happy...
	try
	{
		this->name = "";
		this->volumeStr = "";
		this->color = "";
	}
	catch (...)
	{
		return;
	}
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
void Send::CollectData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, int sendIndex, const std::string& trackAddress, const bool& dump)
{
	std::ostringstream stream;
	stream << trackAddress << "send/" << sendIndex << "/";
	const std::string sendAddress = stream.str();

	// Is enabled?
	bool isMuted;
	GetTrackSendUIMute(track, sendIndex, &isMuted);
	this->enabled = Collectors::CollectIntValue(ss, (sendAddress + "active").c_str(), this->enabled, isMuted ? 0 : 1, dump);

	// Get the name
	constexpr int LENGTH = 20;
	char name[LENGTH] = {};
	DISABLE_WARNING_ARRAY_POINTER_DECAY
	const bool result = GetTrackSendName(track, sendIndex, name, LENGTH);
	const std::string newName = result ? name : "";
	this->name = Collectors::CollectStringValue(ss, (sendAddress + "name").c_str(), this->name, newName, dump);

	// Get the volume
	const double volDB = GetSendVolume(track, sendIndex, ReaperUtils::GetCursorPosition(project));
	this->volume = Collectors::CollectDoubleValue(ss, (sendAddress + "volume").c_str(), this->volume, DB2SLIDER(volDB) / 1000.0, dump);
	this->volumeStr = Collectors::CollectStringValue(ss, (sendAddress + "volume/str").c_str(), this->volumeStr, Collectors::FormatDB(volDB).c_str(), dump);

	// Get the color of the destination track
	MediaTrack* receiveTrack = static_cast<MediaTrack*> (GetSetTrackSendInfo(track, 0, sendIndex, "P_DESTTRACK", nullptr));
	int red = -1, green = -1, blue = -1;
	const int nativeColor = GetTrackColor(receiveTrack);
	if (nativeColor != 0)
		ColorFromNative(nativeColor & 0xFEFFFFFF, &red, &green, &blue);
	this->color = Collectors::CollectStringValue(ss, (sendAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


double Send::GetSendVolume(MediaTrack* track, int sendCounter, double position) const noexcept
{
	const char* sendType = "<VOLENV";
	if (GetMediaTrackInfo_Value(track, "I_AUTOMODE") > 0)
	{
		DISABLE_WARNING_NO_C_STYLE_CONVERSION
		TrackEnvelope* envelope = static_cast<TrackEnvelope*> (GetSetTrackSendInfo(track, 0, sendCounter, "P_ENV", (void*)sendType));
		if (envelope != nullptr)
		{
			// It seems there is always a send envelope, even if not active.
			// Therefore, check if the envelope is active
			for (int i = 0; i < CountTrackEnvelopes(track); i++)
			{
				const TrackEnvelope* te = GetTrackEnvelope(track, i);
				if (envelope == te)
					return ReaperUtils::ValueToDB(ReaperUtils::GetEnvelopeValueAtPosition(envelope, position));
			}
		}
	}
	return ReaperUtils::ValueToDB(GetTrackSendInfo_Value(track, 0, sendCounter, "D_VOL"));
}
