// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "WrapperGSL.h"
#include "SceneProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
SceneProcessor::SceneProcessor(Model &aModel) noexcept : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void SceneProcessor::Process(std::deque<std::string> &path) noexcept
{
	if (path.empty())
		return;
	const char *part = safeGet(path, 0);

	ReaProject *project = ReaperUtils::GetProject();

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char *cmd = safeGet(path, 1);

	const std::vector<int> scenes = Marker::GetRegions(project);
	if (index < 0 || index >= gsl::narrow_cast<int>(scenes.size()))
		return;
	const int sceneID = scenes.at(index);

	const bool isLaunch = std::strcmp(cmd, "launch") == 0;
	if (std::strcmp(cmd, "select") == 0 || isLaunch)
	{
		double position, endPosition;
		const int result = EnumProjectMarkers2(project, sceneID, nullptr, &position, &endPosition, nullptr, nullptr);
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
		// Note: This method seems not to support Undo ...
		DeleteProjectMarkerByIndex(project, sceneID);
		// ... at least we can change the state, so the document state gets updated
		Undo_OnStateChange2(project, "Delete region.");
		return;
	}
}
