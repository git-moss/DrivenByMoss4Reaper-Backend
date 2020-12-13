// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "WrapperGSL.h"
#include "GrooveProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
GrooveProcessor::GrooveProcessor(Model &aModel) noexcept : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void GrooveProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();

	const char* cmd = safeGet(path, 0);

	if (std::strcmp(cmd, "active") == 0)
	{
		int swingmodeInOutOptional = value > 0 ? 1 : 0;
		GetSetProjectGrid(project, true, nullptr, &swingmodeInOutOptional, nullptr);
		return;
	}

	if (std::strcmp(cmd, "amount") == 0)
	{
		double swingamtInOutOptional = value;
		GetSetProjectGrid(project, true, nullptr, nullptr, &swingamtInOutOptional);
		return;
	}
}
