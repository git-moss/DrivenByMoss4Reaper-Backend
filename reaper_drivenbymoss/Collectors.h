// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_COLLECTORS_H_
#define _DBM_COLLECTORS_H_

#include <cstring>
#include <iomanip>
#include <sstream>

#include "ReaDebug.h"


/**
 * Helper functions for checking changed attributes and formatting them as pseudo OSC messages.
 */
class Collectors
{
public:
	static std::string CollectStringValue(std::ostringstream& ss, const std::string& command, const std::string& currentValue, const std::string& newValue, const bool& dump)
	{
		if (currentValue.compare(newValue) != 0 || dump)
		{
			// Note: this needs to use c_str(), otherwise additional \0 are appended as well
			// which prevents that the string is sent
			ss << command << " " << newValue.c_str() << "\n";
		}
		return newValue;
	}


	static int CollectIntValue(std::ostringstream& ss, const std::string& command, const int& currentValue, const int& newValue, const bool& dump)
	{
		if (currentValue != newValue || dump)
			ss << command << " " << newValue << "\n";
		return newValue;
	}


	static double CollectDoubleValue(std::ostringstream& ss, const std::string& command, const double& currentValue, const double& newValue, const bool& dump)
	{
		if (std::fabs(currentValue - newValue) > 0.0000000001 || dump)
			ss << command << " " << newValue << "\n";
		return newValue;
	}


	static void CollectStringArrayValue(std::ostringstream& ss, const std::string& command, int index, std::vector<std::string>& currentValues, const char* newValue, const bool& dump)
	{
		if ((newValue && std::strcmp(currentValues.at(index).c_str(), newValue) != 0) || dump)
		{
			try
			{
				if (newValue == nullptr)
				{
					ss << command << " " << "" << "\n";
					currentValues.at(index).assign("");
					return;
				}
				ss << command << " " << newValue << "\n";
				currentValues.at(index).assign(newValue);
			}
			catch (const std::out_of_range& oor)
			{
				ReaDebug() << "Out of Range error: " << oor.what();
			}
		}
	}


	static void CollectDoubleArrayValue(std::ostringstream& ss, const std::string& command, int index, std::vector<double>& currentValues, double newValue, const bool& dump)
	{
		try
		{
			if (std::fabs(currentValues.at(index) - newValue) > 0.0000000001 || dump)
			{
				ss << command << " " << newValue << "\n";
				currentValues.at(index) = newValue;
			}
		}
		catch (const std::out_of_range& oor)
		{
			ReaDebug() << "Out of Range error: " << oor.what();
		}
	}


	static void CollectIntArrayValue(std::ostringstream& ss, const std::string& command, int index, std::vector<int>& currentValues, int newValue, const bool& dump)
	{
		try
		{
			if (currentValues.at(index) != newValue || dump)
			{
				ss << command << " " << newValue << "\n";
				currentValues.at(index) = newValue;
			}
		}
		catch (const std::out_of_range& oor)
		{
			ReaDebug() << "Out of Range error: " << oor.what();
		}
	}


	static std::string FormatColor(int red, int green, int blue)
	{
		std::ostringstream ss;
		ss << red << " " << green << " " << blue;
		return ss.str();
	}


	static std::string FormatDB(double value)
	{
		if (value == -150)
			return "-inf dB";
		std::ostringstream stream;
		stream << std::fixed << std::setprecision(1);
		if (value >= 0)
			stream << "+";
		stream << value << " dB";
		return stream.str();
	}


	static std::string FormatPan(double value)
	{
		if (std::abs(value) < 0.001)
			return "C";
		std::ostringstream stream;
		if (value < 0)
			stream << static_cast<int>(value * -100) << "L";
		else
			stream << static_cast<int>(value * 100) << "R";
		return stream.str();
	}
};

#endif /* _DBM_COLLECTORS_H_ */
