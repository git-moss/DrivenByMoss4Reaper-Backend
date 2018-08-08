// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>
#include <cmath>
#include "Model.h"


/**
 * Constructor.
 */
Model::Model(FunctionExecutor &aFunctionExecutor) :
	trackVolume(this->trackBankSize, 0),
	trackPan(this->trackBankSize, 0),
	trackSendVolume(this->trackBankSize, std::vector<double>(sendBankSize, 0)),
	functionExecutor(aFunctionExecutor)
{
	// Intentionally empty
}


/**
 * Convert continuous double value to dB.
 *
 * @param x The value to convert
 * @return The value converted to dB
 */
double Model::ValueToDB(double x) noexcept
{
	if (x < 0.0000000298023223876953125)
		return -150;
	// Added extra parenthesis necessary to distinct from Windows define version 
	return (std::max)(-150.0, std::log(x) * 8.6858896380650365530225783783321);
}


/**
 * Convert a dB value to a continuous double value.
 *
 * @param x The value to convert
 * @return The converted value
 */
double Model::DBToValue(double x) noexcept
{
	double val = x * 0.11512925464970228420089957273422;
	val = std::exp(val);
	return val;
}
