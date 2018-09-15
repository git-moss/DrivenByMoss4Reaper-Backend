// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "MarkerProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
MarkerProcessor::MarkerProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void MarkerProcessor::Process(std::string command, std::deque<std::string> &path)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	ReaProject *project = ReaperUtils::GetProject();

	if (std::strcmp(part, "add") == 0)
	{
		const double position = GetPlayPosition2Ex(project);
		std::stringstream markerName;
		markerName << "Marker " << (this->model.markerCount + 1);
		AddProjectMarker(project, false, position, 0, markerName.str().c_str(), 0);
		return;
	}

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char *cmd = path.at(1).c_str();

	const bool isLaunch = std::strcmp(cmd, "launch") == 0;
	if (std::strcmp(cmd, "select") == 0 || isLaunch)
	{
		double position;
		int result = EnumProjectMarkers2(project, index, nullptr, &position, nullptr, nullptr, nullptr);
		if (result)
		{
			SetEditCurPos2(project, position, true, true);
			if (isLaunch && (GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
		}
	}
	else if (std::strcmp(cmd, "remove") == 0)
	{
		DeleteProjectMarkerByIndex(project, index);
	}
}
