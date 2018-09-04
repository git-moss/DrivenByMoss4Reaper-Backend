// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <vector>
#include <mutex>

#include "reaper_plugin_functions.h"
#undef max
#undef min

#include "FunctionExecutor.h"
#include "Track.h"


/**
 * Encapsulates the cached status of a Reaper project.
 */
class Model
{
public:
	const int sendBankSize{ 8 };
	const int deviceBankSize{ 8 };
	const int parameterBankSize{ 8 };
	const int markerBankSize{ 8 };

	double masterVolume{ 0 };
	double masterPan{ 0 };

	int trackCount{ 0 };

	int deviceSelected{ 0 };
	int deviceParamBankSelected{ 0 };
	int deviceParamBankSelectedTemp{ 0 };
	int deviceBankOffset{ 0 };
	int deviceParamCount{ 0 };
	int deviceParamBankOffset{ 0 };
	int deviceExpandedType{ 3 };
	int deviceCount{ 0 };

	int markerBankOffset{ 0 };
	int markerCount{ 0 };


	Model(FunctionExecutor &functionExecutor);

	void AddFunction(std::function<void(void)> f)
	{
		functionExecutor.AddFunction(f);
	};

	Track *GetTrack(const int index);

private:
	FunctionExecutor & functionExecutor;
	std::vector<Track *> tracks;
	std::mutex tracklock;
};
