// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "MastertrackProcessor.h"
#include "ReaperUtils.h"


/**
* Constructor.
*
* @param aModel The model
*/
MastertrackProcessor::MastertrackProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void MastertrackProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.empty())
		return;
	const char *cmd = path.at(0).c_str();

	MediaTrack *track = GetMasterTrack(ReaperUtils::GetProject());
	if (std::strcmp(cmd, "select") == 0)
	{
		SetOnlyTrackSelected(track);
		SetMixerScroll(track);
		this->model.deviceSelected = 0;
		this->model.deviceParamBankSelectedTemp = 0;
	}
	else if (std::strcmp(cmd, "solo") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_SOLO", value);
	}
	else if (std::strcmp(cmd, "mute") == 0)
	{
		SetMediaTrackInfo_Value(track, "B_MUTE", value);
	}
	else
		Process(command, path, static_cast<double>(value));
}


/** {@inheritDoc} */
void MastertrackProcessor::Process(std::string command, std::deque<std::string> &path, double value)
{
	if (path.empty())
		return;
	const char *cmd = path.at(0).c_str();
	MediaTrack *track = GetMasterTrack(ReaperUtils::GetProject());
	if (std::strcmp(cmd, "volume") == 0)
	{
		// Touch not supported            
		if (path.size() == 1)
		{
			this->model.masterVolume = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			SetMediaTrackInfo_Value(track, "D_VOL", this->model.masterVolume);
		}
	}
	else if (strcmp(cmd, "pan") == 0)
	{
		// Touch not supported            
		if (path.size() == 1)
		{
			this->model.masterPan = value * 2 - 1;
			SetMediaTrackInfo_Value(track, "D_PAN", this->model.masterPan);
		}
	}
}
