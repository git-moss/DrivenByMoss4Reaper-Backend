// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "reaper_plugin_functions.h"
#undef max
#undef min
#include "FunctionExecutor.h"
#include "OscParser.h"
#include "JvmManager.h"
#include "DataCollector.h"


/**
 * Surface implementation.
 */
class DrivenByMossSurface : public IReaperControlSurface
{
public:
	DrivenByMossSurface() noexcept;
	~DrivenByMossSurface();

	OscParser &GetOscParser() noexcept
	{
		return oscParser;
	}

	std::string CollectData(bool dump)
	{
		return this->dataCollector.CollectData(dump);
	};

	const char *GetTypeString() override
	{
		return "DrivenByMoss4Reaper";
	}

	const char *GetDescString() override
	{
		return "DrivenByMoss4Reaper - Supports lot's of surfaces...";
	}

	const char *GetConfigString() override
	{
		// String must not be empty or otherwise the surface is not instantiated on startup
		return "empty";
	}

	void Run() override
	{
		functionExecutor.ExecuteFunctions();
	}

	void SetTrackListChange() override
	{
		// TODO Use notification functions to update the model. On Java call only send the changed data and dont retrieve it
		// ShowConsoleMsg("SetTrackListChange\n");
	}

	void SetSurfaceVolume(MediaTrack *trackid, double volume) override
	{
		// ShowConsoleMsg("SetSurfaceVolume\n");
	}

	void SetSurfacePan(MediaTrack *trackid, double pan) override
	{
		// ShowConsoleMsg("SetSurfacePan\n");
	}

	void SetSurfaceMute(MediaTrack *trackid, bool mute) override
	{
		// ShowConsoleMsg("SetSurfaceMute\n");
	}

	void SetSurfaceSelected(MediaTrack *trackid, bool selected) override
	{
		// ShowConsoleMsg("SetSurfaceSelected\n");
	}

	void SetSurfaceSolo(MediaTrack *trackid, bool solo) override
	{
		// trackid==master means "any solo"
		// ShowConsoleMsg("SetSurfaceSolo\n");
	}

	void SetSurfaceRecArm(MediaTrack *trackid, bool recarm) override
	{
		// ShowConsoleMsg("SetSurfaceRecArm\n");
	}

	void SetPlayState(bool play, bool pause, bool rec) override
	{
		// ShowConsoleMsg("SetPlayState\n");
	}

	void SetRepeatState(bool rep) override
	{
		// ShowConsoleMsg("SetRepeatState\n");
	}

	void SetTrackTitle(MediaTrack *trackid, const char *title) override
	{
		// ShowConsoleMsg("SetTrackTitle\n");
	}

	bool GetTouchState(MediaTrack *trackid, int isPan) override
	{
		// ShowConsoleMsg("GetTouchState\n");
		return false;
	}

	void SetAutoMode(int mode) override
	{
		// ShowConsoleMsg("SetAutoMode\n");
	}


	void ResetCachedVolPanStates() override
	{
		// ShowConsoleMsg("ResetCachedVolPanStates\n");
	}


	void OnTrackSelection(MediaTrack *trackid) override
	{
		// ShowConsoleMsg("OnTrackSelection\n");
	}

private:
	FunctionExecutor functionExecutor;
	Model model;
	OscParser oscParser{ model };
	DataCollector dataCollector{ model };
};
