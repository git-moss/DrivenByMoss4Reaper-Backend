// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to markers.
 */
class MarkerProcessor : public OscProcessor
{
public:
	MarkerProcessor(Model &model);

	void Process(std::deque<std::string> &path) override;
};
