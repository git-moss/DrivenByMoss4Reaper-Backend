// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <vector>
#include "reaper_plugin_functions.h"


/**
 * Encapsulates the cached status of a Reaper project.
 */
class Model
{
public:
	const int trackBankSize{ 8 };
	const int sendBankSize{ 8 };
	const int deviceBankSize{ 8 };
	const int parameterBankSize{ 8 };

	double masterVolume{ 0 };
	double masterPan{ 0 };

	int trackBankOffset{ 0 };
	int trackSelection{ 0 };
	int trackCount{ 0 };
	std::vector<double> trackVolume;
	std::vector<double> trackPan;
	std::vector<std::vector<double>> trackSendVolume;

	int deviceSelected{ 0 };
	int deviceParamBankSelected{ 0 };
	int deviceParamBankSelectedTemp{ 0 };
	int deviceBankOffset{ 0 };
	int deviceParamCount{ 0 };
	int deviceParamBankOffset{ 0 };
	int deviceExpandedType{ 3 };
	int deviceCount{ 0 };


	Model();

	double ValueToDB(double x) noexcept;
	double DBToValue(double x) noexcept;

	ReaProject * GetProject() noexcept
	{
		// Current project
		const int projectID = -1;
		return EnumProjects(projectID, nullptr, 0);
	};
};

