// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_DRIVENBYMOSSSURFACE_H_
#define _DBM_DRIVENBYMOSSSURFACE_H_

#include <mutex>

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
	bool isInfrastructureUp{ false };
	bool isShutdown{ false };


	DrivenByMossSurface(std::unique_ptr<JvmManager>& aJvmManager);
	DrivenByMossSurface(const DrivenByMossSurface&) = delete;
	DrivenByMossSurface& operator=(const DrivenByMossSurface&) = delete;
	DrivenByMossSurface(DrivenByMossSurface&&) = delete;
	DrivenByMossSurface& operator=(DrivenByMossSurface&&) = delete;
	~DrivenByMossSurface();

	OscParser& GetOscParser() noexcept
	{
		return this->oscParser;
	}

	DataCollector& GetDataCollector() noexcept
	{
		return this->dataCollector;
	}

	const char* GetTypeString() noexcept override;
	const char* GetDescString() noexcept override;
	const char* GetConfigString() noexcept override;

	void Run() override;

	void SetTrackListChange() noexcept override;
	void SetSurfaceVolume(MediaTrack* trackid, double volume) noexcept override;
	void SetSurfacePan(MediaTrack* trackid, double pan) noexcept override;
	void SetSurfaceMute(MediaTrack* trackid, bool mute) noexcept override;
	void SetSurfaceSelected(MediaTrack* trackid, bool selected) noexcept override;
	void SetSurfaceSolo(MediaTrack* trackid, bool solo) noexcept override;
	void SetSurfaceRecArm(MediaTrack* trackid, bool recarm) noexcept override;
	void SetPlayState(bool play, bool pause, bool rec) noexcept override;
	void SetRepeatState(bool rep) noexcept override;
	void SetTrackTitle(MediaTrack* trackid, const char* title) noexcept override;
	bool GetTouchState(MediaTrack* trackid, int isPan) override;
	void SetAutoMode(int mode) noexcept override;
	void ResetCachedVolPanStates() noexcept override;
	void OnTrackSelection(MediaTrack* trackid) noexcept override;

private:
	std::unique_ptr<JvmManager>& jvmManager;
	FunctionExecutor functionExecutor;
	Model model;
	OscParser oscParser{ model };
	DataCollector dataCollector{ model };
	bool updateModel{false};
	std::mutex startInfrastructureMutex;

	std::string CollectData(bool dump)
	{
		return this->dataCollector.CollectData(dump, this->oscParser.GetActionProcessor());
	};
};


#endif /* _DBM_DRIVENBYMOSSSURFACE_H_ */