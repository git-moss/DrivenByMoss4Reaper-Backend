// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "Collectors.h"
#include "Parameter.h"


/**
 * Constructor.
 */
Parameter::Parameter()
{
	// Intentionally empty
}


/**
 * Destructor.
 */
Parameter::~Parameter()
{
	// Intentionally empty
}


/**
 * Collect the (changed) parameter data.
 *
 * @param ss The stream where to append the formatted data
 * @param oscPath The start of the OSC path to use
 * @param track The track to which the device belongs
 * @param deviceIndex The index of the device to which the parameters belong
 * @param parameterIndex The index of the parameter
 * @param parameterCount The number of all parameters
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Parameter::CollectData(std::stringstream &ss, const char *oscPath, MediaTrack *track, int deviceIndex, int parameterIndex, int parameterCount, const bool &dump)
{
	std::stringstream das;
	das << oscPath << parameterIndex << "/";
	std::string paramAddress = das.str();

    constexpr int LENGTH = 20;
	char name[LENGTH];
	bool result = TrackFX_GetParamName(track, deviceIndex, parameterIndex, name, LENGTH);
	this->name = Collectors::CollectStringValue(ss, (paramAddress + "name").c_str(), this->name, result ? name : "", dump);
	const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, parameterIndex);
	this->value = Collectors::CollectDoubleValue(ss, (paramAddress + "value").c_str(), this->value, paramValue, dump);
	result = TrackFX_FormatParamValueNormalized(track, deviceIndex, parameterIndex, paramValue, name, LENGTH);
	this->valueStr = Collectors::CollectStringValue(ss, (paramAddress + "value/str").c_str(), this->valueStr, result ? name : "", dump);
}
