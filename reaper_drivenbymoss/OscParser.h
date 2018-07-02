// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <iterator>

#include "reaper_plugin_functions.h"
#include "Model.h"
#include "OscProcessor.h"
#include "JvmManager.h"


/**
 * Parse and execute the received OSC style commands.
 */
class OscParser
{
public:
	OscParser(Model *model);
	~OscParser();

	virtual void Process(const std::string &command) const;
	virtual void Process(const std::string &command, const char *value) const;
	virtual void Process(const std::string &command, const int &value) const;
	virtual void Process(const std::string &command, const double &value) const;

private:
	std::map<std::string, OscProcessor *> processors;

	std::deque<std::string> split(const std::string &path) const;
	OscProcessor *GetProcessor(const std::string &command) const;
};
