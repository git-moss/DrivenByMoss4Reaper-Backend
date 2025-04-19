// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "WrapperGSL.h"
#include "SceneProcessor.h"
#include "ReaperUtils.h"
#include "ReaDebug.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
SceneProcessor::SceneProcessor(Model& aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void SceneProcessor::Process(std::deque<std::string>& path)
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	ReaProject* project = ReaperUtils::GetProject();

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char* cmd = SafeGet(path, 1);

	const std::vector<int> regions = Marker::GetRegions(project);
	if (index < 0 || index >= gsl::narrow_cast<int>(regions.size()))
		return;
	const int sceneID = regions.at(index);
	const std::unique_ptr<Marker>& scene = this->model.GetRegion(sceneID);

	const bool isLaunch = std::strcmp(cmd, "launch") == 0;
	if (std::strcmp(cmd, "select") == 0 || isLaunch)
	{
		SetEditCurPos2(project, scene->position, true, true);
		GetSet_LoopTimeRange2(project, true, true, &scene->position, &scene->endPosition, false);
		if (isLaunch && (GetPlayStateEx(project) & 1) == 0)
			CSurf_OnPlay();
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

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		DuplicateScene(project, sceneID, scene.get());
		return;
	}
}


/** {@inheritDoc} */
void SceneProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	ReaProject* project = ReaperUtils::GetProject();

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char* cmd = SafeGet(path, 1);

	const std::vector<int> scenes = Marker::GetRegions(project);
	if (index < 0 || index >= gsl::narrow_cast<int>(scenes.size()))
		return;

	try
	{
		const int sceneID = scenes.at(index);
		const std::unique_ptr<Marker>& scene = this->model.GetRegion(sceneID);

		if (std::strcmp(cmd, "color") == 0)
		{
			int red{ 0 };
			int green{ 0 };
			int blue{ 0 };
			try
			{
				std::cmatch result{};
				if (!std::regex_search(value.c_str(), result, colorPattern))
					return;
				red = std::atoi(result.str(1).c_str());
				green = std::atoi(result.str(2).c_str());
				blue = std::atoi(result.str(3).c_str());
			}
			catch (...)
			{
				return;
			}

			Undo_BeginBlock2(project);
			SetProjectMarker4(project, scene->markerOrRegionIndex, true, scene->position, scene->endPosition, "", ColorToNative(red, green, blue) | 0x1000000, 0);
			Undo_EndBlock2(project, "Change region color", UNDO_STATE_ALL);
			return;
		}

		if (std::strcmp(cmd, "name") == 0)
		{
			Undo_BeginBlock2(project);
			SetProjectMarker4(project, scene->markerOrRegionIndex, true, scene->position, scene->endPosition, value.c_str(), scene->colorNumber ? scene->colorNumber | 0x1000000 : 0, value.length() == 0 ? 1 : 0);
			Undo_EndBlock2(project, "Rename region", UNDO_STATE_ALL);
			return;
		}
	}
	catch (...)
	{
		// Scene index access is checked before
	}
}


void SceneProcessor::DuplicateScene(ReaProject* project, const int sceneID, Marker* scene)
{
	Undo_BeginBlock2(project);

	// Set the time range to the region
	SetEditCurPos2(project, scene->position, true, true);
	GetSet_LoopTimeRange2(project, true, true, &scene->position, &scene->endPosition, false);

	// Item: Select all items in current time selection
	Main_OnCommandEx(40717, 0, project);
	// Time selection : Insert empty space at time selection(moving later items)
	Main_OnCommandEx(40200, 0, project);
	// Item: Copy items to time selection, trim / loop to fit
	Main_OnCommandEx(41319, 0, project);

	// Get the new position of the region
	double newPosition;
	double newEndPosition;
	EnumProjectMarkers2(project, sceneID, nullptr, &newPosition, &newEndPosition, nullptr, nullptr);

	// Move the region back to its' original position
	SetProjectMarkerByIndex2(project, sceneID, true, scene->position, scene->endPosition, scene->markerOrRegionIndex, scene->name.c_str(), 0, 0);

	// Create a new region for the copied clips
	try
	{
		std::string newName = scene->name;
		if (newName.length() == 0)
		{
			std::ostringstream buffer;
			buffer << scene->markerOrRegionIndex;
			newName = buffer.str();
		}
		newName = newName + " - Copy";
		AddProjectMarker2(project, true, newPosition, newEndPosition, newName.c_str(), 0, scene->colorNumber);
	}
	catch (const std::exception& e)
	{
		ReaDebug() << "ERROR: Could not add marker: " << e.what();
	}

	Undo_EndBlock2(project, "Duplicate region", UNDO_STATE_ALL);
}
