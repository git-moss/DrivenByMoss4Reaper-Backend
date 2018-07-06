// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <vector>


class Model
{
public:
	const int trackBankSize = 8;
	const int sendBankSize = 8;
	const int deviceBankSize = 8;
	const int parameterBankSize = 8;

	double masterVolume = 0;
	double masterPan = 0;

	int trackBankOffset = 0;
	int trackSelection = 0;
	int trackCount = 0;
	std::vector<double> trackVolume;
	std::vector<double> trackPan;
	std::vector<std::vector<double>> trackSendVolume;

	int deviceSelected = -1;
	int deviceParamBankSelected = -1;
	int deviceParamBankSelectedTemp = -1;
	int deviceBankOffset = 0;
	int deviceParamCount;
	int deviceParamBankOffset = 0;
	int deviceExpandedType = -1;
	int deviceExpandedTypeTemp = -1;
	int deviceCount = -1;


	Model();

	double ValueToDB(double x);
	double DBToValue(double x);
};

