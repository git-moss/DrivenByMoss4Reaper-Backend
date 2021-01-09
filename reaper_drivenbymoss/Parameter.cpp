// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "CodeAnalysis.h"
#include "Collectors.h"
#include "Parameter.h"


/**
 * Constructor.
 *
 * @param prefixPath The start path to use for the OSC identificer
 * @param index The index of the parameter
 */
Parameter::Parameter(const char* prefixPath, const int index) noexcept : parameterIndex(index)
{
	try
	{
		std::ostringstream das;
		DISABLE_WARNING_CAN_THROW
			das << prefixPath << index << "/";
		const std::string paramAddress = das.str();

		this->addressName = paramAddress + "name";
		this->addressValue = paramAddress + "value";
		this->addressValueStr = paramAddress + "value/str";
	}
	catch (const std::exception& ex)
	{
		ReaDebug() << "Could not create parameter addresses: " << ex.what();
	}
}


/**
 * Collect the (changed) parameter data.
 *
 * @param ss The stream where to append the formatted data
 * @param track The track to which the device belongs
 * @param deviceIndex The index of the device to which the parameters belong
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Parameter::CollectData(std::ostringstream& ss, MediaTrack* track, const int& deviceIndex, const bool& dump)
{
	CollectData(ss, track, deviceIndex, this->parameterIndex, dump);
}


/**
 * Collect the (changed) parameter data.
 *
 * @param ss The stream where to append the formatted data
 * @param track The track to which the device belongs
 * @param deviceIndex The index of the device to which the parameters belong
 * @param paramIndex The index of the parameter
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Parameter::CollectData(std::ostringstream& ss, MediaTrack* track, const int& deviceIndex, const int& paramIndex, const bool& dump)
{
	// The warning about array pointer decay is ignored because it cannot be fixed since we have to use the available Reaper function
	constexpr int LENGTH = 20;
	char nameBuf[LENGTH];

	DISABLE_WARNING_ARRAY_POINTER_DECAY
		bool result = TrackFX_GetParamName(track, deviceIndex, paramIndex, nameBuf, LENGTH);
	const std::string newName{ result ? nameBuf : "" };
	this->name = Collectors::CollectStringValue(ss, this->addressName, this->name, newName, dump);

	const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, paramIndex);
	const bool valueHasChanged = this->value != paramValue;
	this->value = Collectors::CollectDoubleValue(ss, this->addressValue, this->value, paramValue, dump);

	if (valueHasChanged)
	{
		DISABLE_WARNING_ARRAY_POINTER_DECAY
			result = TrackFX_FormatParamValueNormalized(track, deviceIndex, paramIndex, paramValue, nameBuf, LENGTH);
		const std::string newValue{ result ? nameBuf : "" };
		this->valueStr = Collectors::CollectStringValue(ss, this->addressValueStr, this->valueStr, newValue, dump);
	}
}
