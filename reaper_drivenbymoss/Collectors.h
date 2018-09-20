#pragma once

#include <iomanip>
#include <sstream>

#include "ReaDebug.h"


/**
 * Helper functions for checking changed attributes and formatting them as pseudo OSC messages.
 */
class Collectors
{
public:
	static const char *CollectStringValue(std::stringstream &ss, const char *command, std::string currentValue, const char *newValue, const bool &dump)
	{
		if ((newValue && std::strcmp(currentValue.c_str(), newValue) != 0) || dump)
			ss << command << " " << newValue << "\n";
		return newValue;
	}


	static int CollectIntValue(std::stringstream &ss, const char *command, int currentValue, const int newValue, const bool &dump)
	{
		if (currentValue != newValue || dump)
			ss << command << " " << newValue << "\n";
		return newValue;
	}


	static double CollectDoubleValue(std::stringstream &ss, const char *command, double currentValue, const double newValue, const bool &dump)
	{
		if (fabs(currentValue - newValue) > 0.0000000001 || dump)
			ss << command << " " << newValue << "\n";
		return newValue;
	}


	static void CollectStringArrayValue(std::stringstream &ss, const char *command, int index, std::vector<std::string> &currentValues, const char *newValue, const bool &dump)
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
			catch (const std::out_of_range &oor)
			{
				ReaDebug() << "Out of Range error: " << oor.what();
			}
		}
	}


	static void CollectDoubleArrayValue(std::stringstream &ss, const char *command, int index, std::vector<double> &currentValues, double newValue, const bool &dump)
	{
		try
		{
			if (std::fabs(currentValues.at(index) - newValue) > 0.0000000001 || dump)
			{
				ss << command << " " << newValue << "\n";
				currentValues.at(index) = newValue;
			}
		}
		catch (const std::out_of_range &oor)
		{
			ReaDebug() << "Out of Range error: " << oor.what();
		}
	}


	static void CollectIntArrayValue(std::stringstream &ss, const char *command, int index, std::vector<int> &currentValues, int newValue, const bool &dump)
	{
		try
		{
			if (currentValues.at(index) != newValue || dump)
			{
				ss << command << " " << newValue << "\n";
				currentValues.at(index) = newValue;
			}
		}
		catch (const std::out_of_range &oor)
		{
			ReaDebug() << "Out of Range error: " << oor.what();
		}
	}


	static std::string FormatColor(int red, int green, int blue)
	{
		std::stringstream ss;
		ss << red << " " << green << " " << blue;
		return ss.str();
	}


	static std::string FormatDB(double value)
	{
		if (value == -150)
			return "-inf dB";
		std::stringstream stream;
		stream << std::fixed << std::setprecision(1);
		if (value >= 0)
			stream << "+";
		stream << value << " dB";
		return stream.str();
	}


	static std::string FormatPan(double value)
	{
		if (abs(value) < 0.001)
			return "C";
		std::stringstream stream;
		if (value < 0)
			stream << static_cast<int>(value * -100) << "L";
		else
			stream << static_cast<int>(value * 100) << "R";
		return stream.str();
	}
};