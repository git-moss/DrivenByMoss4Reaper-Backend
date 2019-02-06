// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

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
	const int sendBankSize{ 8 };
	const int deviceBankSize{ 8 };
	const int markerBankSize{ 8 };

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


	Model(FunctionExecutor &functionExecutor);

	void AddFunction(std::function<void(void)> f)
	{
		functionExecutor.AddFunction(f);
	};

	Track *GetTrack(const int index);
	Marker *GetMarker(const int index);
	Marker *GetRegion(const int index);
	Parameter *GetParameter(const int index);

	void SetDump()
	{
		dumplock.lock();
		this->dump = true;
		dumplock.unlock();
	}

	bool ShouldDump()
	{
		bool d{ false };
		dumplock.lock();
		if (this->dump)
		{
			this->dump = false;
			d = true;
		}
		dumplock.unlock();
		return d;
	}

private:
	FunctionExecutor & functionExecutor;
	std::vector<Track *> tracks;
	std::vector<Marker *> markers;
	std::vector<Marker *> regions;
	std::vector<Parameter *> parameters;
	std::mutex tracklock;
	std::mutex markerlock;
	std::mutex regionlock;
	std::mutex parameterlock;
	std::mutex dumplock;
	bool dump{false};
};
