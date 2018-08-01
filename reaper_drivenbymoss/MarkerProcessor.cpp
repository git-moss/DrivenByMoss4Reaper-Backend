// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "MarkerProcessor.h"


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

	if (std::strcmp(part, "bank") == 0)
	{
		const char *cmd = path.at(1).c_str();
		if (std::strcmp(cmd, "+") == 0)
		{
			if (this->model.markerBankOffset < this->model.markerCount)
				this->model.markerBankOffset += this->model.markerBankSize;
		}
		else if (std::strcmp(cmd, "-") == 0)
		{
			if (this->model.markerBankOffset > 0)
				this->model.markerBankOffset -= this->model.markerBankSize;
		}
		return;
	}
	
	ReaProject *project = this->model.GetProject();

	if (std::strcmp(part, "add") == 0)
	{
		// TODO AddProjectMarker(project, false, double pos, 0, const char* name, int wantidx);
		return;
	}

	if (path.size() < 2)
		return;

	const int index = this->model.markerBankOffset + atoi(part) - 1;
	const char *cmd = path.at(1).c_str();

	if (std::strcmp(cmd, "select") == 0)
	{
		double position;
		int result = EnumProjectMarkers2(project, index, nullptr, &position, nullptr, nullptr, nullptr);
		if (result)
			SetEditCurPos2(project, position, true, true);
	}
	else if (std::strcmp(cmd, "launch") == 0)
	{
		double position;
		int result = EnumProjectMarkers2(project, index, nullptr, &position, nullptr, nullptr, nullptr);
		if (result)
		{
			SetEditCurPos2(project, position, true, true);
			if ((GetPlayStateEx(project) & 1) == 0)
				CSurf_OnPlay();
		}
	}
	else if (std::strcmp(cmd, "remove") == 0)
	{
		DeleteProjectMarkerByIndex(project, index);
	}
}
