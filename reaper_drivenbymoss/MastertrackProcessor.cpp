// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
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
void MastertrackProcessor::Process(std::deque<std::string> &path, int value) noexcept
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
		return;
	}
	
	if (std::strcmp(cmd, "solo") == 0)
	{
		SetMediaTrackInfo_Value(track, "I_SOLO", value);
		return;
	}
	
	if (std::strcmp(cmd, "mute") == 0)
	{
		SetMediaTrackInfo_Value(track, "B_MUTE", value);
		return;
	}

	if (std::strcmp(cmd, "autotrim") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 0);
		return;
	}

	if (std::strcmp(cmd, "autoread") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 1);
		return;
	}

	if (std::strcmp(cmd, "autotouch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 2);
		return;
	}

	if (std::strcmp(cmd, "autowrite") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 3);
		return;
	}

	if (std::strcmp(cmd, "autolatch") == 0)
	{
		if (value > 0)
			SetMediaTrackInfo_Value(track, "I_AUTOMODE", 4);
		return;
	}

	Process(path, static_cast<double>(value));
}


/** {@inheritDoc} */
void MastertrackProcessor::Process(std::deque<std::string> &path, double value) noexcept
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
		return;
	}
	
	if (strcmp(cmd, "pan") == 0)
	{
		// Touch not supported            
		if (path.size() == 1)
		{
			this->model.masterPan = value * 2 - 1;
			SetMediaTrackInfo_Value(track, "D_PAN", this->model.masterPan);
		}
		return;
	}
}

/** {@inheritDoc} */
void MastertrackProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;
	const char* cmd = path.at(0).c_str();
	MediaTrack* track = GetMasterTrack(ReaperUtils::GetProject());
	ReaProject* project = ReaperUtils::GetProject();

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfTrack(project, track, value);
		return;
	}
}

void MastertrackProcessor::SetColorOfTrack(ReaProject* project, MediaTrack* track, std::string value)
{
	if (track == nullptr)
		return;

	std::cmatch result;
	if (!std::regex_search(value.c_str(), result, colorPattern))
		return;
	int red = std::atoi(result.str(1).c_str());
	int green = std::atoi(result.str(2).c_str());
	int blue = std::atoi(result.str(3).c_str());

	Undo_BeginBlock2(project);
	// Note: SetTrackColor is not working for the master track
	SetMediaTrackInfo_Value(track, "I_CUSTOMCOLOR", ColorToNative(red, green, blue));
	Undo_EndBlock2(project, "Set master track color", 0);
}