// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "WrapperGSL.h"
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
void MarkerProcessor::Process(std::deque<std::string> &path)
{
	if (path.empty())
		return;
	const char *part = SafeGet(path, 0);

	ReaProject *project = ReaperUtils::GetProject();

	if (std::strcmp(part, "add") == 0)
	{
		Undo_BeginBlock2(project);
		PreventUIRefresh(1);
		const double position = ReaperUtils::GetCursorPosition(project);
		std::ostringstream markerName;
		markerName << "Marker " << (this->model.markerCount + 1);
		AddProjectMarker(project, false, position, 0, markerName.str().c_str(), 0);
		PreventUIRefresh(-1);
		Undo_EndBlock2(project, "Add project marker", UNDO_STATE_ALL);
		return;
	}

	if (path.size() < 2)
		return;

	const int index = atoi(part);
	const char *cmd = SafeGet(path, 1);

	const std::vector<int> markers = Marker::GetMarkers(project);
	if (index < 0 || index >= gsl::narrow_cast<int> (markers.size()))
		return;
	const int markerID = markers.at(index);

	const bool isLaunch = std::strcmp(cmd, "launch") == 0;
	if (std::strcmp(cmd, "select") == 0 || isLaunch)
	{
		double position;
		const int result = EnumProjectMarkers2(project, markerID, nullptr, &position, nullptr, nullptr, nullptr);
		if (result)
		{
			SetEditCurPos2(project, position, true, true);
			if (isLaunch && (GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
		}
		return;
	}
	
	if (std::strcmp(cmd, "remove") == 0)
	{
		Undo_BeginBlock2(project);
		DeleteProjectMarkerByIndex(project, markerID);
		Undo_EndBlock2(project, "Delete project marker", UNDO_STATE_ALL);
		return;
	}
}
