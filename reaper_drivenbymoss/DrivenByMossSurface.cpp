// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt


#include "ReaDebug.h"
#include "DrivenByMossSurface.h"

// Singleton, deleted from Reaper
DrivenByMossSurface* surfaceInstance = nullptr;


/**
 * Constructor.
 */
DISABLE_WARNING_NO_REF_TO_UNIQUE_PTR
DrivenByMossSurface::DrivenByMossSurface(std::unique_ptr<JvmManager>& aJvmManager) : isShutdown(false), jvmManager(aJvmManager), model(functionExecutor), updateModel(false)
{
	ReaDebug::setModel(&model);
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	this->isShutdown = true;

	// Null global variables
	ReaDebug::setModel(nullptr);
	surfaceInstance = nullptr;
}


const char* DrivenByMossSurface::GetTypeString() noexcept
{
	return "DrivenByMoss4Reaper";
}


const char* DrivenByMossSurface::GetDescString() noexcept
{
	return "DrivenByMoss4Reaper - Supports lot's of surfaces...";
}


const char* DrivenByMossSurface::GetConfigString() noexcept
{
	// String must not be empty or otherwise the surface is not instantiated on startup
	return "empty";
}


/**
 * The control surface callback function for updating the device. Called 30x/sec or so.
 */
void DrivenByMossSurface::Run()
{
	if (this->jvmManager == nullptr || !this->jvmManager->IsRunning() || this->isShutdown)
		return;

	try
	{
		this->functionExecutor.ExecuteFunctions();
	}
	catch (const std::exception& ex)
	{
		ReaDebug() << "Could not update device: " << ex.what();
	}
	catch (...)
	{
		ReaDebug() << "Could not update device.";
	}

	this->oscParser.GetActionProcessor().CheckActionSelection();

	// Only update each 2nd call (about 60ms)
	this->updateModel = !this->updateModel;
	if (!this->updateModel)
		return;

	std::string data = this->CollectData(this->model.ShouldDump());
	if (data.length() > 0)
		this->jvmManager.get()->UpdateModel(data);
}


void DrivenByMossSurface::SetTrackListChange() noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceVolume(MediaTrack* trackid, double volume) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfacePan(MediaTrack* trackid, double pan) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceMute(MediaTrack* trackid, bool mute) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSelected(MediaTrack* trackid, bool selected) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSolo(MediaTrack* trackid, bool solo) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceRecArm(MediaTrack* trackid, bool recarm) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetPlayState(bool play, bool pause, bool rec) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetRepeatState(bool rep) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetTrackTitle(MediaTrack* trackid, const char* title) noexcept
{
	// Not used
}

bool DrivenByMossSurface::GetTouchState(MediaTrack* trackid, int isPan)
{
	if (trackid == GetMasterTrack(ReaperUtils::GetProject()))
		return isPan ? model.isMasterPanTouch : model.isMasterVolumeTouch;

	const int position = static_cast<int>(GetMediaTrackInfo_Value(trackid, "IP_TRACKNUMBER")) - 1;
	if (position < 0)
		return false;

	const std::unique_ptr<Track>& trackPtr = model.GetTrack(position);
	return isPan ? trackPtr->isPanTouch : trackPtr->isVolumeTouch;
}

void DrivenByMossSurface::SetAutoMode(int mode) noexcept
{
	// Not used
}


void DrivenByMossSurface::ResetCachedVolPanStates() noexcept
{
	// Not used
}


void DrivenByMossSurface::OnTrackSelection(MediaTrack* trackid) noexcept
{
	// Not used
}
