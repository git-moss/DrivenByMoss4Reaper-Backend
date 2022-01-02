// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
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


	Parameter(const char* prefixPath, const int index) noexcept;

	void CollectData(std::ostringstream &ss, MediaTrack *track, const int& deviceIndex, const bool &dump);
	void CollectData(std::ostringstream& ss, MediaTrack* track, const int& deviceIndex, const int& paramIndex, const bool& dump);
	void ClearData(std::ostringstream& ss, const bool& dump);

private:
	const int parameterIndex;

	std::string addressName;
	std::string addressValue;
	std::string addressValueStr;
};

#endif /* _DBM_PARAMETER_H_ */