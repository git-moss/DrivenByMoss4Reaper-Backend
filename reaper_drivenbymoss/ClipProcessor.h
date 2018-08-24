// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "OscProcessor.h"


/**
 * Processes all commands related to clips.
 */
class ClipProcessor : public OscProcessor
{
public:
	ClipProcessor(Model &model);

	void Process(std::string command, std::deque<std::string> &path) override;

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		Process(command, path, static_cast<double> (value));
	};

	void Process(std::string command, std::deque<std::string> &path, double value) override;
	void Process(std::string command, std::deque<std::string> &path, const std::string &value) override;

private:
	void SetColorOfClip(ReaProject *project, std::string value);
};
