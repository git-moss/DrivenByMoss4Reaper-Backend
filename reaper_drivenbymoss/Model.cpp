// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>
#include <cmath>

#include "Model.h"



double Model::ValueToDB(double x)
{
	if (x < 0.0000000298023223876953125)
		return -150;
	// Added extra
	return (std::max)(-150.0, std::log(x) * 8.6858896380650365530225783783321);
}


double Model::DBToValue(double x)
{
	double val = x * 0.11512925464970228420089957273422;
	val = std::exp(val);
	return val;
}
