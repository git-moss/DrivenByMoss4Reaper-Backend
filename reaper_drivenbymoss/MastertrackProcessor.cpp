// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "MastertrackProcessor.h"
#include "ReaperUtils.h"
#include "DrivenByMossSurface.h"

extern DrivenByMossSurface* surfaceInstance;


/**
* Constructor.
*
* @param aModel The model
*/
MastertrackProcessor::MastertrackProcessor(Model& aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void MastertrackProcessor::Process(std::deque<std::string>& path, int value) noexcept
{
	if (path.empty())
		return;
	const char* cmd = SafeGet(path, 0);
	MediaTrack* track = GetMasterTrack(ReaperUtils::GetProject());

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
void MastertrackProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;
	const char* cmd = SafeGet(path, 0);
	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetMasterTrack(project);

	if (std::strcmp(cmd, "volume") == 0)
	{
		if (path.size() == 1)
		{
			this->model.masterVolume = ReaperUtils::DBToValue(SLIDER2DB(value * 1000.0));
			const double newVolume = SetTrackUIVolume(track, this->model.masterVolume, false, false, IGNORE_GROUP_FLAGS);
			if (!this->model.isMasterVolumeTouch)
				CSurf_SetSurfaceVolume(track, newVolume, surfaceInstance);
			return;
		}

		const char* touchCmd = SafeGet(path, 1);
		if (std::strcmp(touchCmd, "touch") == 0)
			this->model.isMasterVolumeTouch = value > 0;
		return;
	}

	if (strcmp(cmd, "pan") == 0)
	{
		if (path.size() == 1)
		{
			this->model.masterPan = value * 2 - 1;
			const double newPan = SetTrackUIPan(track, this->model.masterPan, false, false, IGNORE_GROUP_FLAGS);
			if (!this->model.isMasterPanTouch)
				CSurf_SetSurfacePan(track, newPan, nullptr);
			return;
		}

		const char* touchCmd = SafeGet(path, 1);
		if (std::strcmp(touchCmd, "touch") == 0)
			this->model.isMasterPanTouch = value > 0;
		return;
	}

	// Parse master FX parameter value
	if (std::strcmp(cmd, "param") == 0)
	{
		if (path.empty())
			return;
		const int fxParamNo = atoi(SafeGet(path, 1));
		int fxindexOut;
		int parmidxOut;
		if (!GetTCPFXParm(project, track, fxParamNo, &fxindexOut, &parmidxOut))
			return;
		if (std::strcmp(SafeGet(path, 2), "value") == 0)
			TrackFX_SetParamNormalized(track, fxindexOut, parmidxOut, value);
		return;
	}
}

/** {@inheritDoc} */
void MastertrackProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;
	const char* cmd = SafeGet(path, 0);
	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetMasterTrack(project);

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfTrack(project, track, value);
		return;
	}
}

void MastertrackProcessor::SetColorOfTrack(ReaProject* project, MediaTrack* track, const std::string& value) noexcept
{
	if (track == nullptr)
		return;

	int red{ 0 };
	int green{ 0 };
	int blue{ 0 };
	try
	{
		std::cmatch result;
		if (!std::regex_search(value.c_str(), result, colorPattern))
			return;
		red = std::atoi(result.str(1).c_str());
		green = std::atoi(result.str(2).c_str());
		blue = std::atoi(result.str(3).c_str());
	}
	catch (...)
	{
		return;
	}

	PreventUIRefresh(1);
	Undo_BeginBlock2(project);
	// Note: SetTrackColor is not working for the master track
	SetMediaTrackInfo_Value(track, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | SET_COLOR);
	Undo_EndBlock2(project, "Set master track color", UNDO_STATE_ALL);
	PreventUIRefresh(-1);
}