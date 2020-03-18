// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_PARAMETER_H_
#define _DBM_PARAMETER_H_

#include <string>

#include "ReaperUtils.h"


/**
 * Encapsulates Reaper C functions for parameters.
 */
class Parameter
{
public:
	std::string name;
	double value{ 0.0 };
	std::string valueStr;


	Parameter() noexcept;

	void CollectData(std::stringstream &ss, const char *oscPath, MediaTrack *track, int deviceIndex, int parameterIndex, int parameterCount, const bool &dump);
};

#endif /* _DBM_PARAMETER_H_ */