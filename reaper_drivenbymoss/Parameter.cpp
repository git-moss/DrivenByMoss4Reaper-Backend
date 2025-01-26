// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
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
	constexpr int LENGTH = 60;
	std::string nameBuf(LENGTH, 0);
	char* nameBufPointer = &*nameBuf.begin();

	bool result = TrackFX_GetParamName(track, deviceIndex, paramIndex, nameBufPointer, LENGTH);
	const std::string newName{ result ? nameBuf : "" };
	this->name = Collectors::CollectStringValue(ss, this->addressName, this->name, newName, dump);

	// Note: this seems to already respect the envelope!
	const double paramValue = TrackFX_GetParamNormalized(track, deviceIndex, paramIndex);
	const bool valueHasChanged = this->value != paramValue;
	this->value = Collectors::CollectDoubleValue(ss, this->addressValue, this->value, paramValue, dump);

	if (dump || valueHasChanged)
	{
		const double realValue = TrackFX_GetParam(track, deviceIndex, paramIndex, nullptr, nullptr);
		DISABLE_WARNING_ARRAY_POINTER_DECAY
		result = TrackFX_FormatParamValue(track, deviceIndex, paramIndex, realValue, nameBufPointer, LENGTH);
		const std::string newValue{ result ? nameBuf : "" };
		this->valueStr = Collectors::CollectStringValue(ss, this->addressValueStr, this->valueStr, newValue, dump);
	}
}


/**
 * Sets all attributes back to their initial (empty) values.
 *
 * @param ss The stream where to append the formatted data
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Parameter::ClearData(std::ostringstream& ss, const bool& dump)
{
	this->name = Collectors::CollectStringValue(ss, this->addressName, this->name, "", dump);
	this->value = Collectors::CollectDoubleValue(ss, this->addressValue, this->value, 0.0, dump);
	this->valueStr = Collectors::CollectStringValue(ss, this->addressValueStr, this->valueStr, "", dump);
}
