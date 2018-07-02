// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"
#include "Model.h"


class MastertrackProcessor : public OscProcessor
{
public:
	MastertrackProcessor(Model &model);

	virtual void Process(std::string command, std::deque<std::string> &path);
	virtual void Process(std::string command, std::deque<std::string> &path, int value);
	virtual void Process(std::string command, std::deque<std::string> &path, double value);

private:
	Model & model;
};