// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_REAPERUTILS_H_
#define _DBM_REAPERUTILS_H_

#include <algorithm>
#include <cmath>
#include <vector>

#include "CodeAnalysis.h"
#include "WrapperReaperFunctions.h"

static constexpr double VU_BOTTOM{ 60.0 };
static constexpr double VU_TOP{ 6.0 };
static constexpr double VU_CLIP{ 0.9912109375 };


/**
 * Helper functions for Reaper functions.
 */
class ReaperUtils
{
public:
	static HWND mainWindowHandle;


	/**
	 * Get the current project.
	 *
	 * @return The current project
	 */
	static ReaProject* GetProject() noexcept
	{
		// Current project
		constexpr int projectID = -1;
		return EnumProjects(projectID, nullptr, 0);
	};


	/**
	 * Get the value of a configuration variable of an integer type. Returns the value
	 * of a project configuration if present, otherwise get it from the global settings.
	 *
	 * For a list of variable names see:
	 * https://mespotin.uber.space/Mespotine/Ultraschall/Reaper_Config_Variables.html
	 *
	 * @param variableName The name of the variable
	 * @return The value, returns -1 if not present
	 */
	static int GetIntConfigValue(const char* variableName) noexcept
	{
		void* resultPtr = GetConfigValue(variableName);
		return resultPtr == nullptr ? -1 : *((int*)resultPtr);
	}


	/**
	 * Get the value of a configuration variable of an integer type. Returns the value
	 * of a project configuration if present, otherwise get it from the global settings.
	 *
	 * For a list of variable names see:
	 * https://mespotin.uber.space/Mespotine/Ultraschall/Reaper_Config_Variables.html
	 *
	 * @param variableName The name of the variable
	 * @param defaultValue The value to return if the variable is not present
	 * @return The value, returns the given defaultValue if not present
	 */
	static int GetIntConfigValue(const char* variableName, int defaultValue) noexcept
	{
		void* resultPtr = GetConfigValue(variableName);
		return resultPtr == nullptr ? defaultValue : *((int*)resultPtr);
	}


	/**
	 * Get the pointer to the value of a configuration variable. Returns the value of a
	 * project configuration if present, otherwise get it from the global settings.
	 *
	 * For a list of variable names see:
	 * https://mespotin.uber.space/Mespotine/Ultraschall/Reaper_Config_Variables.html
	 *
	 * @param variableName The name of the variable
	 * @return The pointer to the value
	 */
	static void* GetConfigValue(const char* variableName) noexcept
	{
		int szOut = 0;
		void* resultPtr = nullptr;
		// First check if the variable is present in the project settings
		const int offset = projectconfig_var_getoffs(variableName, &szOut);
		if (offset)
			resultPtr = projectconfig_var_addr(EnumProjects(-1, nullptr, 0), offset);
		else
		{
			// Otherwise get it from the global settings
			resultPtr = get_config_var(variableName, &szOut);
		}
		return resultPtr;
	}


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
		DISABLE_WARNING_DEREF_INVALID_POINTER
		return (std::max)(-150.0, std::log(x) * 8.6858896380650365530225783783321);
	}


	/**
	 * Scale the VU value to the Reaper meter display range of -60dB to 0.0dB to [0..1].
	 * 
	 * @param vuValue The VU value retrieved with Track_GetPeakInfo
	 * @return The VU value as dB in the range of [0..1]
	 */
	static double ValueToVURange(const double vuValue) noexcept
	{
		const double dbValue = ReaperUtils::ValueToDB(vuValue);

		// Cut above +6dB which is the highest value displayed in Reaper
		if (dbValue >= VU_TOP)
			return 1.0;

		// Cut below -60 dB which is the lowest value displayed in Reaper
		if (dbValue < -VU_BOTTOM)
			return 0.0;

		// Scale the clip region [0..+6] to [0.9912..1]
		if (dbValue > 0)
			return VU_CLIP + (dbValue / VU_TOP);

		// Scale [-60..0] to [0..0.9912]
		return (dbValue + VU_BOTTOM) / VU_BOTTOM * VU_CLIP;
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


	/**
	 * Calculates the value of the given envelope at the given timeline position.
	 *
	 * @param envelope The envelope
	 * @param position A position on the timeline
	 * @return The calculated value
	 */
	static double GetEnvelopeValueAtPosition(TrackEnvelope* envelope, double position) noexcept
	{
		const int sampleRate = ReaperUtils::GetIntConfigValue("projsrate");
		double value;
		Envelope_Evaluate(envelope, position, sampleRate, 1, &value, nullptr, nullptr, nullptr);
		return ScaleFromEnvelopeMode(GetEnvelopeScalingMode(envelope), value);
	}


	/**
	 * Get the current positon of the play cursor on the timeline.
	 *
	 * @param project A project
	 * @return The position in seconds
	 */
	static double GetCursorPosition(ReaProject* project) noexcept
	{
		return (GetPlayStateEx(project) & 1) > 0 ? GetPlayPositionEx(project) : GetCursorPositionEx(project);
	}


	static HWND GetArrangeWnd() noexcept
	{
		constexpr int mainArrange = 0x000003E8;
		return GetDlgItem(mainWindowHandle, mainArrange);
	}
};

#endif /* _DBM_REAPERUTILS_H_ */