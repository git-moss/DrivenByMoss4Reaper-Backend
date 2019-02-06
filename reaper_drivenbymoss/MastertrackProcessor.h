// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"
#include "Model.h"


/**
 * Processes all commands related to the master track.
 */
class MastertrackProcessor : public OscProcessor
{
public:
	MastertrackProcessor(Model &model);

	void Process(std::deque<std::string> &path, int value) override;
	void Process(std::deque<std::string> &path, double value) override;
};
