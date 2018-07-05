// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"
#include "Model.h"


class ClipProcessor : public OscProcessor
{
public:
	ClipProcessor(Model *model);

	// TODO
	//virtual void Process(std::string command, std::deque<std::string> &path);
	virtual void Process(std::string command, std::deque<std::string> &path, int value);
	//virtual void Process(std::string command, std::deque<std::string> &path, double value);
	virtual void Process(std::string command, std::deque<std::string> &path, std::string value);

private:
	Model * model;


	void SetColorOfClip(ReaProject *project, std::string value);
};

