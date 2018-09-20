// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>


/**
 * Encapsulates Reaper C functions for parameters.
 */
class Parameter
{
public:
	std::string name;
	double value{ 0.0 };
	std::string valueStr;


	Parameter();
	virtual ~Parameter();

	void CollectData(std::stringstream &ss, MediaTrack *track, int deviceIndex, int parameterIndex, int parameterCount, const bool &dump);
};

