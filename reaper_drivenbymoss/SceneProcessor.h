// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to scenes.
 */
class SceneProcessor : public OscProcessor
{
public:
	SceneProcessor(Model &model);

	void Process(std::string command, std::deque<std::string> &path) override;
};
