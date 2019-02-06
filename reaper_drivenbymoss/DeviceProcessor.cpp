// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "DeviceProcessor.h"
#include "ReaperUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model
 */
DeviceProcessor::DeviceProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string> &path)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	if (std::strcmp(part, "page") == 0)
	{
		part = path.at(1).c_str();
		if (std::strcmp(part, "+") == 0)
		{
			SetDeviceSelection(this->model.deviceBankOffset + this->model.deviceSelected + this->model.deviceBankSize);
		}
		else if (std::strcmp(part, "-") == 0)
		{
			SetDeviceSelection(this->model.deviceBankOffset + this->model.deviceSelected - this->model.deviceBankSize);
		}
		return;
	}

	if (std::strcmp(part, "+") == 0)
	{
		SetDeviceSelection(this->model.deviceBankOffset + this->model.deviceSelected + 1);
		return;
	}

	if (std::strcmp(part, "-") == 0)
	{
		SetDeviceSelection(this->model.deviceBankOffset + this->model.deviceSelected - 1);
		return;
	}

	ReaProject *project = ReaperUtils::GetProject();
	const int fx = atoi(part) - 1;

	MediaTrack *track = GetSelectedTrack(project, 0);
	if (track == nullptr)
		return;

	const char *cmd = path.at(1).c_str();
	if (std::strcmp(cmd, "remove") == 0)
	{
		TrackFX_Delete(track, fx);
		return;
	}

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		TrackFX_CopyToTrack(track, fx, track, fx + 1, false);
		return;
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string> &path, int value)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	ReaProject *project = ReaperUtils::GetProject();
	MediaTrack *track = GetSelectedTrack(project, 0);
	if (track == nullptr)
		return;

	const int selDevice = this->model.deviceBankOffset + this->model.deviceSelected;

	if (std::strcmp(part, "selected") == 0)
	{
		this->model.deviceSelected = value - 1;
		return;
	}
	
	if (std::strcmp(part, "bypass") == 0)
	{
		if (selDevice >= 0)
			TrackFX_SetEnabled(track, selDevice, value > 0 ? 0 : 1);
		return;
	}
	
	if (std::strcmp(part, "window") == 0)
	{
		if (selDevice < 0)
			return;
		bool open = value > 0;
		if (open)
			TrackFX_Show(track, selDevice, this->model.deviceExpandedType);
		TrackFX_SetOpen(track, selDevice, open);
		return;
	}
	
	if (std::strcmp(part, "expand") == 0)
	{
		if (selDevice < 0)
			return;
		const bool isOpen = TrackFX_GetOpen(track, selDevice);
		int expandedType = value > 0 ? 1 : 3;
		this->model.deviceExpandedType = expandedType;
		if (!isOpen)
			return;
		TrackFX_SetOpen(track, selDevice, 0);
		TrackFX_Show(track, selDevice, expandedType);
		return;
	}
	
	if (std::strcmp(part, "page") == 0)
	{
		part = path.at(1).c_str();
		if (std::strcmp(part, "selected") == 0)
		{
			this->model.deviceSelected = (value - 1) * this->model.deviceBankSize;
		}
		return;
	}
	
	if (std::strcmp(part, "preset") == 0)
	{
		TrackFX_SetPresetByIndex(track, this->model.deviceBankOffset + this->model.deviceSelected, value);
		return;
	}
	
	if (std::strcmp(part, "param") == 0)
	{
		Process(path, static_cast<double> (value));
		return;
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string> &path, double value)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	ReaProject *project = ReaperUtils::GetProject();
	MediaTrack *track = GetSelectedTrack(project, 0);
	if (track == nullptr)
		return;

	const int selDevice = this->model.deviceBankOffset + this->model.deviceSelected;

	if (std::strcmp(part, "param") == 0)
	{
		const int paramNo = atoi(path.at(1).c_str());
		if (std::strcmp(path.at(2).c_str(), "value") == 0)
			TrackFX_SetParamNormalized(track, selDevice, paramNo, value);
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string> &path, const std::string &value)
{
	if (path.empty())
		return;
	const char *part = path.at(0).c_str();

	ReaProject *project = ReaperUtils::GetProject();
	MediaTrack *track = GetSelectedTrack(project, 0);

	if (std::strcmp(part, "add") == 0)
	{
		int position = TrackFX_AddByName(track, value.c_str(), false, -1);
		if (position < 0)
			return;
		const int insert = atoi(path.at(1).c_str());
		TrackFX_CopyToTrack(track, position, track, insert, true);
	}
}


void DeviceProcessor::SetDeviceSelection(int position)
{
	const int pos = (std::min)((std::max)(0, position), this->model.deviceCount - 1);
	this->model.deviceSelected = pos % this->model.deviceBankSize;
	this->model.deviceBankOffset = static_cast<int>(std::floor(pos / this->model.deviceBankSize) * this->model.deviceBankSize);
}
