// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
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

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		DuplicateScene(project, sceneID);
		return;
	}
}

void SceneProcessor::DuplicateScene(ReaProject* project, const int sceneID)
{
	double position, endPosition;
	const char* name;
	int markerID;
	int color;
	const int result = EnumProjectMarkers3(project, sceneID, nullptr, &position, &endPosition, &name, &markerID, &color);
	if (result)
	{
		Undo_BeginBlock2(project);

		// Set the time range to the region
		SetEditCurPos2(project, position, true, true);
		GetSet_LoopTimeRange2(project, true, true, &position, &endPosition, false);

		// Item: Select all items in current time selection
		Main_OnCommandEx(40717, 0, project);
		// Time selection : Insert empty space at time selection(moving later items)
		Main_OnCommandEx(40200, 0, project);
		// Item: Copy items to time selection, trim / loop to fit
		Main_OnCommandEx(41319, 0, project);

		// Get the new position of the region
		double newPosition, newEndPosition;
		EnumProjectMarkers2(project, sceneID, nullptr, &newPosition, &newEndPosition, nullptr, nullptr);

		// Move the region back to its' original position
		SetProjectMarkerByIndex2(project, sceneID, true, position, endPosition, markerID, name, 0, 0);

		// Create a new region for the copied clips
		try
		{
			std::string newName = name;
			if (newName.length() == 0)
			{
				std::ostringstream buffer;
				buffer << markerID;
				newName = buffer.str();
			}
			newName = newName + " - Copy";
			AddProjectMarker2(project, true, newPosition, newEndPosition, newName.c_str(), 0, color);
		}
		catch (const std::exception& e)
		{
			ReaDebug() << "ERROR: Could not add marker: " << e.what();
		}

		Undo_EndBlock2(project, "Duplicate region", UNDO_STATE_ALL);
	}
}
