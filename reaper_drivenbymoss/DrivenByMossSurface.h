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

extern JvmManager *jvmManager;


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

	void Run() override;
	void SetTrackListChange() override;
	void SetSurfaceVolume(MediaTrack *trackid, double volume) override;
	void SetSurfacePan(MediaTrack *trackid, double pan) override;
	void SetSurfaceMute(MediaTrack *trackid, bool mute) override;
	void SetSurfaceSelected(MediaTrack *trackid, bool selected) override;
	void SetSurfaceSolo(MediaTrack *trackid, bool solo) override;
	void SetSurfaceRecArm(MediaTrack *trackid, bool recarm) override;
	void SetPlayState(bool play, bool pause, bool rec) override;
	void SetRepeatState(bool rep) override;
	void SetTrackTitle(MediaTrack *trackid, const char *title) override;
	bool GetTouchState(MediaTrack *trackid, int isPan) override;
	void SetAutoMode(int mode) override;
	void ResetCachedVolPanStates() override;
	void OnTrackSelection(MediaTrack *trackid) override;

private:
	FunctionExecutor functionExecutor;
	Model model;
	OscParser oscParser{ model };
	DataCollector dataCollector{ model };


	std::string CollectData(bool dump)
	{
		return this->dataCollector.CollectData(dump);
	};
};
