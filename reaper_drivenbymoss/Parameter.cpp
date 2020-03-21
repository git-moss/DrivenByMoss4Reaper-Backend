// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "CodeAnalysis.h"
#include "Collectors.h"
#include "Parameter.h"


/**
 * Constructor.
 */
Parameter::Parameter() noexcept
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
void Parameter::CollectData(std::ostringstream& ss, const char* oscPath, MediaTrack* track, int deviceIndex, int parameterIndex, int parameterCount, const bool& dump)
{
	std::ostringstream das;
	das << oscPath << parameterIndex << "/";
	const std::string paramAddress = das.str();

	// The warning about array pointer decay is ignored because it cannot be fixed since we have to use the available Reaper function
	constexpr int LENGTH = 20;
	char nameBuf[LENGTH];

	DISABLE_WARNING_PUSH
	DISABLE_WARNING_ARRAY_POINTER_DECAY
	bool result = TrackFX_GetParamName(track, deviceIndex, parameterIndex, nameBuf, LENGTH);
	this->name = Collectors::CollectStringValue(ss, (paramAddress + "name").c_str(), this->name, result ? nameBuf : "", dump);
	const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, parameterIndex);
	this->value = Collectors::CollectDoubleValue(ss, (paramAddress + "value").c_str(), this->value, paramValue, dump);
	result = TrackFX_FormatParamValueNormalized(track, deviceIndex, parameterIndex, paramValue, nameBuf, LENGTH);
	this->valueStr = Collectors::CollectStringValue(ss, (paramAddress + "value/str").c_str(), this->valueStr, result ? nameBuf : "", dump);
	DISABLE_WARNING_POP
}
