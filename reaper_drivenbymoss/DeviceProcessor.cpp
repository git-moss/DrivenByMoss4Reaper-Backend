// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>

#include "DeviceProcessor.h"


DeviceProcessor::DeviceProcessor(Model *aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


void DeviceProcessor::Process(std::string command, std::deque<std::string> &path)
{
	if (path.empty())
		return;
	const char *part = path[0].c_str();

	ReaProject *project = this->GetProject();
	MediaTrack *track = GetTrack(project, this->model->trackBankOffset + this->model->trackSelection);
	int selDevice = this->model->deviceBankOffset + this->model->deviceSelected;

	if (std::strcmp(part, "page") == 0)
	{
		part = path[1].c_str();
		if (std::strcmp(part, "+") == 0)
		{
			SetDeviceSelection(this->model->deviceBankOffset + this->model->deviceSelected + this->model->deviceBankSize);
		}
		else if (std::strcmp(part, "-") == 0)
		{
			SetDeviceSelection(this->model->deviceBankOffset + this->model->deviceSelected - this->model->deviceBankSize);
		}
	}
	else if (std::strcmp(part, "+") == 0)
	{
		SetDeviceSelection(this->model->deviceBankOffset + this->model->deviceSelected + 1);
	}
	else if (std::strcmp(part, "-") == 0)
	{
		SetDeviceSelection(this->model->deviceBankOffset + this->model->deviceSelected - 1);
	}
	else if (std::strcmp(part, "param") == 0)
	{
		part = path[1].c_str();

		if (std::strcmp(part, "bank") == 0)
		{
			part = path[2].c_str();

			if (std::strcmp(part, "+") == 0)
			{
				if ((this->model->deviceParamBankSelected + this->model->parameterBankSize) * this->model->parameterBankSize < this->model->deviceParamCount)
					this->model->deviceParamBankSelectedTemp += this->model->parameterBankSize;
			}
			else if (std::strcmp(part, "-") == 0)
			{
				if (this->model->deviceParamBankSelected >= this->model->parameterBankSize)
					this->model->deviceParamBankSelectedTemp -= this->model->parameterBankSize;
			}
		}
		else if (std::strcmp(part, "+") == 0)
		{
			if ((this->model->deviceParamBankSelected + 1) * this->model->parameterBankSize < this->model->deviceParamCount)
				this->model->deviceParamBankSelectedTemp += 1;
		}
		else if (std::strcmp(part, "-") == 0)
		{
			if (this->model->deviceParamBankSelected >= 1)
				this->model->deviceParamBankSelectedTemp -= 1;
		}
	}
}


void DeviceProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.empty())
		return;
	const char *part = path[0].c_str();

	ReaProject *project = this->GetProject();
	MediaTrack *track = GetTrack(project, this->model->trackBankOffset + this->model->trackSelection);
	if (track == nullptr)
		return;

	int selDevice = this->model->deviceBankOffset + this->model->deviceSelected;

	if (std::strcmp(part, "selected") == 0)
	{
		this->model->deviceSelected = value - 1;
		this->model->deviceParamBankSelectedTemp = 0;
	}
	else if (std::strcmp(part, "bypass") == 0)
	{
		if (selDevice >= 0)
			TrackFX_SetEnabled(track, selDevice, value > 0 ? 0 : 1);
	}
	else if (std::strcmp(part, "window") == 0)
	{
		if (selDevice >= 0)
		{
			if (value > 0)
				TrackFX_Show(track, selDevice, this->model->deviceExpandedType);
			else
				TrackFX_SetOpen(track, selDevice, 0);
		}
	}
	else if (std::strcmp(part, "expand") == 0)
	{
		if (selDevice >= 0)
		{
			bool isOpen = TrackFX_GetOpen(track, selDevice);
			TrackFX_SetOpen(track, selDevice, 0);
			this->model->deviceExpandedTypeTemp = value > 0 ? 1 : 3;
			if (isOpen)
				TrackFX_Show(track, selDevice, this->model->deviceExpandedTypeTemp);
		}
	}
	else if (std::strcmp(part, "page") == 0)
	{
		part = path[1].c_str();
		if (std::strcmp(part, "selected") == 0)
		{
			this->model->deviceSelected = (value - 1) * this->model->deviceBankSize;
		}
	}
	else if (std::strcmp(part, "preset") == 0)
	{
		TrackFX_SetPresetByIndex(track, this->model->deviceBankOffset + this->model->deviceSelected, value);
	}
	else if (std::strcmp(part, "param") == 0)
	{
		part = path[1].c_str();

		if (std::strcmp(part, "bank") == 0)
		{
			part = path[2].c_str();

			if (std::strcmp(part, "selected") == 0)
			{
				this->model->deviceParamBankSelectedTemp = value - 1;
			}
		}
		else
		{
			int paramNo = atoi(path[2].c_str()) - 1;
			if (std::strcmp(path[3].c_str(), "value") == 0)
				TrackFX_SetParamNormalized(track, selDevice, (this->model->deviceParamBankOffset + this->model->deviceParamBankSelected) * this->model->parameterBankSize + paramNo, value);
		}
	}
}


void DeviceProcessor::Process(std::string command, std::deque<std::string> &path, std::string value)
{
	if (path.empty())
		return;
	const char *part = path[0].c_str();

	ReaProject *project = this->GetProject();
	MediaTrack *track = GetTrack(project, this->model->trackBankOffset + this->model->trackSelection);
	int selDevice = this->model->deviceBankOffset + this->model->deviceSelected;

	if (std::strcmp(part, "add") == 0)
	{
		int position = TrackFX_AddByName(track, value.c_str(), false, -1);
		if (position >= 0)
		{
			if (APIExists("SNM_MoveOrRemoveTrackFX"))
			{
				// TODO SNM_MoveOrRemoveTrackFX
				//insert = strToInt(parsePart(line, '/', 3));
				//while (position > insert)
				//{
				//	SNM_MoveOrRemoveTrackFX(track, position, -1);
				//	position -= 1;
				//}
			}
		}
	}
	//else
	//	ShowConsoleMsg(sprintf(#, "Device not found: %s\n", value));
}


void DeviceProcessor::SetDeviceSelection(int position)
{
	int pos = (std::min)((std::max)(0, position), this->model->deviceCount - 1);
	this->model->deviceSelected = pos % this->model->deviceBankSize;
	this->model->deviceBankOffset = (int)std::floor(pos / this->model->deviceBankSize) * this->model->deviceBankSize;
}
