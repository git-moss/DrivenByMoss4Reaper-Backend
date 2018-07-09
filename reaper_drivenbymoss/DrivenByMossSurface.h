// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "reaper_plugin_functions.h"
#include "DrivenByMossExtension.h"
#include "FunctionExecutor.h"


/**
 * Surface implementation.
 */
class DrivenByMossSurface : public IReaperControlSurface
{
public:
	DrivenByMossSurface();
	~DrivenByMossSurface();


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

	virtual void SetTrackListChange()
	{
		// TODO Use notification functions to update the model. On Java call only send the changed data and dont retrieve it
		// ShowConsoleMsg("SetTrackListChange\n");
	}

	virtual void SetSurfaceVolume(MediaTrack *trackid, double volume)
	{
		// ShowConsoleMsg("SetSurfaceVolume\n");
	}

	virtual void SetSurfacePan(MediaTrack *trackid, double pan)
	{
		// ShowConsoleMsg("SetSurfacePan\n");
	}

	virtual void SetSurfaceMute(MediaTrack *trackid, bool mute)
	{
		// ShowConsoleMsg("SetSurfaceMute\n");
	}

	virtual void SetSurfaceSelected(MediaTrack *trackid, bool selected)
	{
		// ShowConsoleMsg("SetSurfaceSelected\n");
	}

	virtual void SetSurfaceSolo(MediaTrack *trackid, bool solo)
	{
		// trackid==master means "any solo"
		// ShowConsoleMsg("SetSurfaceSolo\n");
	}

	virtual void SetSurfaceRecArm(MediaTrack *trackid, bool recarm)
	{
		// ShowConsoleMsg("SetSurfaceRecArm\n");
	}

	virtual void SetPlayState(bool play, bool pause, bool rec)
	{
		// ShowConsoleMsg("SetPlayState\n");
	}

	virtual void SetRepeatState(bool rep)
	{
		// ShowConsoleMsg("SetRepeatState\n");
	}

	virtual void SetTrackTitle(MediaTrack *trackid, const char *title)
	{
		// ShowConsoleMsg("SetTrackTitle\n");
	}

	virtual bool GetTouchState(MediaTrack *trackid, int isPan)
	{
		// ShowConsoleMsg("GetTouchState\n");
		return false;
	}

	virtual void SetAutoMode(int mode)
	{
		// ShowConsoleMsg("SetAutoMode\n");
	}


	virtual void ResetCachedVolPanStates()
	{
		// ShowConsoleMsg("ResetCachedVolPanStates\n");
	}


	virtual void OnTrackSelection(MediaTrack *trackid)
	{
		// ShowConsoleMsg("OnTrackSelection\n");
	}

private:
	FunctionExecutor functionExecutor;
	DrivenByMossExtension extension;
};
