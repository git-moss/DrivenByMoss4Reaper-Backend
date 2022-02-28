// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MODEL_H_
#define _DBM_MODEL_H_

#include <vector>
#include <mutex>

#include "FunctionExecutor.h"
#include "Marker.h"
#include "Track.h"
#include "Parameter.h"


/**
 * Encapsulates the cached status of a Reaper project.
 */
class Model
{
public:
	static const int DEVICE_BANK_SIZE{ 8 };
	static const int MARKER_BANK_SIZE{ 8 };

	double masterVolume{ 0 };
	double masterPan{ 0 };

	int trackCount{ 0 };
	int markerCount{ 0 };
	int sceneCount{ 0 };

	int deviceSelected{ 0 };
	int deviceBankOffset{ 0 };
	int deviceParamCount{ 0 };
	int deviceExpandedType{ 3 };
	int deviceCount{ 0 };
	int eqParamCount{ 0 };
	int userParamCount{ 0 };

	int pinnedTrackIndex{ -1 };


	Model(FunctionExecutor& functionExecutor) noexcept;

	void AddFunction(std::function<void(void)> f) noexcept;

	std::unique_ptr<Track>& GetTrack(const int index) noexcept;
	std::unique_ptr<Marker>& GetMarker(const int index) noexcept;
	std::unique_ptr<Marker>& GetRegion(const int index) noexcept;
	std::unique_ptr<Parameter>& GetParameter(const int index) noexcept;
	std::unique_ptr<Parameter>& GetEqParameter(const int index);
	std::unique_ptr<Parameter>& GetUserParameter(const int index) noexcept;

	void SetDump();
	bool ShouldDump();

	void SetDeviceSelection(int position) noexcept;

private:
	FunctionExecutor& functionExecutor;
	std::vector<std::unique_ptr<Track>> tracks;
	std::vector<std::unique_ptr<Marker>> markers;
	std::vector<std::unique_ptr<Marker>> regions;
	std::vector<std::unique_ptr<Parameter>> parameters;
	std::vector<std::unique_ptr<Parameter>> eqParameters;
	std::vector<std::unique_ptr<Parameter>> userParameters;
	std::mutex tracklock;
	std::mutex markerlock;
	std::mutex regionlock;
	std::mutex parameterlock;
	std::mutex dumplock;
	bool dump{ false };
};

#endif /* _DBM_MODEL_H_ */