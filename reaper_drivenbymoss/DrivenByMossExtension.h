// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "OscParser.h"
#include "JvmManager.h"
#include "DataCollector.h"


/**
 * Encapsulates the main infrastructure for the DrivenByMoss extension.
 */
class DrivenByMossExtension
{
public:
	DrivenByMossExtension(bool enableDebug);

	OscParser &GetOscParser() noexcept
	{
		return oscParser;
	}

	JvmManager &GetJvmManager() noexcept
	{
		return this->jvmManager;
	};

	std::string CollectData(bool dump)
	{
		return this->dataCollector.CollectData(dump);
	};

private:
	Model model{};
	OscParser oscParser{model};
	DataCollector dataCollector{ model };
	JvmManager jvmManager;
};
