// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "ProjectProcessor.h"
#include "ReaperUtils.h"
#include "OscProcessor.h"
#include "SceneProcessor.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
ProjectProcessor::ProjectProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void ProjectProcessor::Process(std::deque<std::string>& path)
{
	if (path.empty())
		return;

	const char* cmd = SafeGet(path, 0);

	if (std::strcmp(cmd, "createSceneFromPlayingLauncherClips") == 0)
	{
		ReaProject* project = ReaperUtils::GetProject();
		const double position = GetCursorPositionEx(project);
		int sceneID;
		GetLastMarkerAndCurRegion(project, position, nullptr, &sceneID);
		if (sceneID >= 0)
		{
			const std::unique_ptr<Marker>& scene = this->model.GetRegion(sceneID);
			SceneProcessor::DuplicateScene(project, sceneID, scene.get());
		}
		return;
	}
};


/** {@inheritDoc} */
void ProjectProcessor::Process(std::deque<std::string>& path, int value)
{
	if (path.empty())
		return;

	const char* cmd = SafeGet(path, 0);

	if (std::strcmp(cmd, "engine") == 0)
	{
		if (value > 0)
			Audio_Init();
		else
			Audio_Quit();
		return;
	}

	if (std::strcmp(cmd, "createScene") == 0)
	{
		ReaProject* project = ReaperUtils::GetProject();
		const std::vector<int> regions = Marker::GetRegions(project);
		const size_t count = regions.size();
		double rgnend{0};
		if (count > 0)
		{
			const int index = regions.at(count - 1);
			EnumProjectMarkers2(project, index, nullptr, nullptr, &rgnend, nullptr, nullptr);
		}

		// Calculate length in seconds of n beats
		double bpmOut;
		GetProjectTimeSignature2(project, &bpmOut, nullptr);
		const double length = static_cast<double>(value) * 60.0 / bpmOut;
		CreateRegion(project, rgnend, length);

		return;
	}
}


void ProjectProcessor::CreateRegion(ReaProject* project, double start, double length) noexcept
{
	Undo_BeginBlock2(project);
	AddProjectMarker(project, true, start, start + length, nullptr, 0);
	Undo_EndBlock2(project, "Add region marker", UNDO_STATE_ALL);
}
