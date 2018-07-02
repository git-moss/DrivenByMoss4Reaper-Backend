// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <cstdlib>
#include <deque>

#include "reaper_plugin_functions.h"


/**
 * Interface to processing commands and executing them on Reaper.
 */
class OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path) {};

	virtual void Process(std::string command, std::deque<std::string> &path, const char *value) {};

	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		if (value == 1)
			this->Process(command, path);
	};

	virtual void Process(std::string command, std::deque<std::string> &path, double value) {};

	virtual void Process(std::string command, std::deque<std::string> &path, float value)
	{
		this->Process(command, path, (double) value);
	};

protected:
	ReaProject * GetProject()
	{
		const int gProjectID = 0;
		// TODO
		ReaProject *project = nullptr;
		return project;
	};
};
