// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <cstdlib>
#include <deque>
#include <regex>

#include "reaper_plugin_functions.h"
#include "Model.h"


/**
 * Interface to processing commands and executing them on Reaper.
 */
class OscProcessor
{
public:
	OscProcessor(Model &aModel) : model (aModel)
	{
		// Intentionally empty
	}

	virtual ~OscProcessor() {};

	virtual void Process(std::string command, std::deque<std::string> &path) {};

	virtual void Process(std::string command, std::deque<std::string> &path, const std::string &value) {};

	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		if (value == 1)
			this->Process(command, path);
	};

	virtual void Process(std::string command, std::deque<std::string> &path, double value) {};

	virtual void Process(std::string command, std::deque<std::string> &path, float value)
	{
		this->Process(command, path, static_cast<double>(value));
	};

protected:
	const std::regex colorPattern{ "RGB\\((\\d+),(\\d+),(\\d+)\\)" };

	Model &model;
};
