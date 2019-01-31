// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "SceneProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
SceneProcessor::SceneProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void SceneProcessor::Process(std::deque<std::string> &path)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	ReaProject *project = ReaperUtils::GetProject();

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char *cmd = path.at(1).c_str();

	const std::vector<int> scenes = Marker::GetRegions(project);
	if (index < 0 || index >= (int)scenes.size())
		return;
	const int sceneID = scenes.at(index);

	const bool isLaunch = std::strcmp(cmd, "launch") == 0;
	if (std::strcmp(cmd, "select") == 0 || isLaunch)
	{
		double position, endPosition;
		int result = EnumProjectMarkers2(project, sceneID, nullptr, &position, &endPosition, nullptr, nullptr);
		if (result)
		{
			SetEditCurPos2(project, position, true, true);
			GetSet_LoopTimeRange2(project, true, true, &position, &endPosition, false);
			if (isLaunch && (GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
		}
		return;
	}

	if (std::strcmp(cmd, "remove") == 0)
	{
		DeleteProjectMarkerByIndex(project, sceneID);
		return;
	}
}
