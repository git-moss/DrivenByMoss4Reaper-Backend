// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "OscProcessor.h"
#include "Model.h"


class DeviceProcessor : public OscProcessor
{
public:
	DeviceProcessor(Model *model);

	virtual void Process(std::string command, std::deque<std::string> &path);
	virtual void Process(std::string command, std::deque<std::string> &path, int value);
	// TODO
	//virtual void Process(std::string command, std::deque<std::string> &path, double value);
	virtual void Process(std::string command, std::deque<std::string> &path, std::string value);

private:
	Model * model;


	void SetDeviceSelection(int position);
};
