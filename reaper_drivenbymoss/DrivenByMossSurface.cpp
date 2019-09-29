// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ReaDebug.h"
#include "DrivenByMossSurface.h"

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

	// Only update each 2nd call (about 60ms)
	this->updateModel = !this->updateModel;
	if (!this->updateModel)
		return;

	std::string data = this->CollectData(this->model.ShouldDump());
	if (data.length() > 0)
		jvmManager->UpdateModel(data);
}


void DrivenByMossSurface::SetTrackListChange()
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceVolume(MediaTrack *trackid, double volume)
{
	// Not used
}

void DrivenByMossSurface::SetSurfacePan(MediaTrack *trackid, double pan)
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceMute(MediaTrack *trackid, bool mute)
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSelected(MediaTrack *trackid, bool selected)
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSolo(MediaTrack *trackid, bool solo)
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceRecArm(MediaTrack *trackid, bool recarm)
{
	// Not used
}

void DrivenByMossSurface::SetPlayState(bool play, bool pause, bool rec)
{
	// Not used
}

void DrivenByMossSurface::SetRepeatState(bool rep)
{
	// Not used
}

void DrivenByMossSurface::SetTrackTitle(MediaTrack *trackid, const char *title)
{
	// Not used
}

bool DrivenByMossSurface::GetTouchState(MediaTrack *trackid, int isPan)
{
	// Not used
	return false;
}

void DrivenByMossSurface::SetAutoMode(int mode)
{
	// Not used
}


void DrivenByMossSurface::ResetCachedVolPanStates()
{
	// Not used
}


void DrivenByMossSurface::OnTrackSelection(MediaTrack *trackid)
{
	// Not used
}
