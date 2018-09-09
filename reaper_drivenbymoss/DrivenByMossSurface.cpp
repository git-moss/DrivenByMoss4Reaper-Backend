// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ReaDebug.h"
#include "DrivenByMossSurface.h"
#include "de_mossgrabers_transformator_TransformatorApplication.h"

extern DrivenByMossSurface *gSurface;


/**
 * Constructor.
 */
DrivenByMossSurface::DrivenByMossSurface() noexcept : model(functionExecutor)
{
	gSurface = this;
	ReaDebug::init(&model);
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	gSurface = nullptr;
}


/**
 * The control surface callback function for updating the device.
 */
void DrivenByMossSurface::Run()
{
	this->functionExecutor.ExecuteFunctions();
	std::string data = this->CollectData(this->model.ShouldDump());
	if (data.length() > 0)
		jvmManager->UpdateModel(data);
}


void DrivenByMossSurface::SetTrackListChange()
{
	ShowConsoleMsg("SetTrackListChange\n");
}

void DrivenByMossSurface::SetSurfaceVolume(MediaTrack *trackid, double volume)
{
	// ShowConsoleMsg("SetSurfaceVolume\n");
}

void DrivenByMossSurface::SetSurfacePan(MediaTrack *trackid, double pan)
{
	// ShowConsoleMsg("SetSurfacePan\n");
}

void DrivenByMossSurface::SetSurfaceMute(MediaTrack *trackid, bool mute)
{
	// ShowConsoleMsg("SetSurfaceMute\n");
}

void DrivenByMossSurface::SetSurfaceSelected(MediaTrack *trackid, bool selected)
{
	// ShowConsoleMsg("SetSurfaceSelected\n");
}

void DrivenByMossSurface::SetSurfaceSolo(MediaTrack *trackid, bool solo)
{
	// trackid==master means "any solo"
	// ShowConsoleMsg("SetSurfaceSolo\n");
}

void DrivenByMossSurface::SetSurfaceRecArm(MediaTrack *trackid, bool recarm)
{
	// ShowConsoleMsg("SetSurfaceRecArm\n");
}

void DrivenByMossSurface::SetPlayState(bool play, bool pause, bool rec)
{
	// ShowConsoleMsg("SetPlayState\n");
}

void DrivenByMossSurface::SetRepeatState(bool rep)
{
	// ShowConsoleMsg("SetRepeatState\n");
}

void DrivenByMossSurface::SetTrackTitle(MediaTrack *trackid, const char *title)
{
	// ShowConsoleMsg("SetTrackTitle\n");
}

bool DrivenByMossSurface::GetTouchState(MediaTrack *trackid, int isPan)
{
	// ShowConsoleMsg("GetTouchState\n");
	return false;
}

void DrivenByMossSurface::SetAutoMode(int mode)
{
	// ShowConsoleMsg("SetAutoMode\n");
}


void DrivenByMossSurface::ResetCachedVolPanStates()
{
	// ShowConsoleMsg("ResetCachedVolPanStates\n");
}


void DrivenByMossSurface::OnTrackSelection(MediaTrack *trackid)
{
	// ShowConsoleMsg("OnTrackSelection\n");
}
