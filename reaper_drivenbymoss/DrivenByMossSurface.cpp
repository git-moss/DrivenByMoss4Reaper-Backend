// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ReaDebug.h"
#include "DrivenByMossSurface.h"

// TODO Only for logging
#include <chrono>
#include <iostream>

/**
 * Constructor.
 */
DrivenByMossSurface::DrivenByMossSurface(std::unique_ptr<JvmManager> & aJvmManager) noexcept : jvmManager(aJvmManager), model(functionExecutor), updateModel(false), isShutdown(false)
{
	ReaDebug::init(&model);
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	this->isShutdown = true;
	this->jvmManager.reset();
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
	// TODO
//	auto start = std::chrono::steady_clock::now();

	if (this->jvmManager == nullptr || this->isShutdown)
		return;

	this->functionExecutor.ExecuteFunctions();

	// Only update each 2nd call (about 60ms)
	this->updateModel = !this->updateModel;
	if (!this->updateModel)
		return;

	std::string data = this->CollectData(this->model.ShouldDump());
	if (data.length() > 0)
		this->jvmManager.get()->UpdateModel(data);

	/*
	 auto end = std::chrono::steady_clock::now();
	 auto diff = end - start;
	
	 std::ostringstream stringStream;
	 stringStream << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
	 std::string copyOfStr = stringStream.str();
	 OutputDebugString (stringToWs(copyOfStr).c_str());*/
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

bool DrivenByMossSurface::GetTouchState(MediaTrack* trackid, int isPan) noexcept
{
	// Not used
	return false;
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
