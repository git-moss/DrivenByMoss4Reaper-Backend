// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once


class Model
{
public:
	double masterVolume = 0;
	double masterPan = 0;

	int deviceSelected = -1;
	int deviceParamBankSelected = -1;
	int deviceParamBankSelectedTemp = -1;


	double ValueToDB(double x);
	double DBToValue(double x);
};

