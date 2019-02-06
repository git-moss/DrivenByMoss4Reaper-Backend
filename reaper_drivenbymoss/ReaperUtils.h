// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "reaper_plugin_functions.h"
#undef max
#undef min


/**
 * Helper functions for Reaper functions.
 */
class ReaperUtils
{
public:
	/**
	 * Get the current project.
	 *
	 * @return The current project
	 */
	static ReaProject *GetProject() noexcept
	{
		// Current project
		const int projectID = -1;
		return EnumProjects(projectID, nullptr, 0);
	};


	/**
	 * Convert continuous double value to dB.
	 *
	 * @param x The value to convert
	 * @return The value converted to dB
	 */
	static double ValueToDB(double x) noexcept
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
	static double DBToValue(double x) noexcept
	{
		double val = x * 0.11512925464970228420089957273422;
		val = std::exp(val);
		return val;
	}
};