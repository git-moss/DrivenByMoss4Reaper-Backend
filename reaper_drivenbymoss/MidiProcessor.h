// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


class MidiProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value);
};
