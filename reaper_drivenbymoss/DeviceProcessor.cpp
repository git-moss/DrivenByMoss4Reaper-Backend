// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "DeviceProcessor.h"
#include "ReaperUtils.h"
#include "StringUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model
 */
DeviceProcessor::DeviceProcessor(Model& aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/**
 * Get the index of the device which is selected on the controller.
 */
int DeviceProcessor::GetDeviceSelection() noexcept
{
	return this->model.deviceBankOffset + this->model.deviceSelected;
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string>& path) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	int devicePosition = this->GetDeviceSelection();

	if (std::strcmp(part, "page") == 0)
	{
		part = SafeGet(path, 1);
		if (std::strcmp(part, "+") == 0)
		{
			devicePosition += this->model.DEVICE_BANK_SIZE;
		}
		else if (std::strcmp(part, "-") == 0)
		{
			devicePosition -= this->model.DEVICE_BANK_SIZE;
		}
		this->model.SetDeviceSelection(devicePosition);
		return;
	}

	if (std::strcmp(part, "+") == 0)
	{
		this->model.SetDeviceSelection(devicePosition + 1);
		return;
	}

	if (std::strcmp(part, "-") == 0)
	{
		this->model.SetDeviceSelection(devicePosition - 1);
		return;
	}

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack2(project, 0, true);
	if (track == nullptr)
		return;

	const int fx = atoi(part) - 1;

	const char* cmd = SafeGet(path, 1);

	if (std::strcmp(cmd, "remove") == 0)
	{
		PreventUIRefresh(1);
		Undo_BeginBlock2(project);
		TrackFX_Delete(track, fx);
		// Make sure a device is selected
		const int max = TrackFX_GetCount(track);
		this->model.SetDeviceSelection(fx < max ? devicePosition : max - 1);
		Undo_EndBlock2(project, "Delete device", UNDO_STATE_FX);
		PreventUIRefresh(-1);
		return;
	}

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		TrackFX_CopyToTrack(track, fx, track, fx + 1, false);
		return;
	}

	if (std::strcmp(cmd, "movePrev") == 0)
	{
		if (fx > 0)
			TrackFX_CopyToTrack(track, fx, track, fx - 1, true);
		return;
	}

	if (std::strcmp(cmd, "moveNext") == 0)
	{
		TrackFX_CopyToTrack(track, fx, track, fx + 1, true);
		return;
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string>& path, int value) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack2(project, 0, true);
	if (track == nullptr)
		return;

	if (std::strcmp(part, "selected") == 0)
	{
		const int fxSel = value - 1;
		if (fxSel >= 0 && fxSel < TrackFX_GetCount(track))
			this->model.SetDeviceSelection(this->model.deviceBankOffset + fxSel);
		return;
	}

	if (std::strcmp(part, "page") == 0)
	{
		part = SafeGet(path, 1);
		if (std::strcmp(part, "selected") == 0)
			this->model.SetDeviceSelection(this->model.deviceBankOffset + (value - 1) * this->model.DEVICE_BANK_SIZE);
		return;
	}

	const int devicePosition = this->GetDeviceSelection();

	if (std::strcmp(part, "preset") == 0)
	{
		TrackFX_SetPresetByIndex(track, devicePosition, value);
		return;
	}

	if (std::strcmp(part, "bypass") == 0)
	{
		if (devicePosition >= 0)
			TrackFX_SetEnabled(track, devicePosition, value > 0 ? 0 : 1);
		return;
	}

	if (std::strcmp(part, "window") == 0)
	{
		if (devicePosition < 0)
			return;
		const bool open = value > 0;
		if (open)
			TrackFX_Show(track, devicePosition, this->model.deviceExpandedType);
		TrackFX_SetOpen(track, devicePosition, open);
		return;
	}

	if (std::strcmp(part, "expand") == 0)
	{
		if (devicePosition < 0)
			return;
		const bool isOpen = TrackFX_GetOpen(track, devicePosition);
		const int expandedType = value > 0 ? 1 : 3;
		this->model.deviceExpandedType = expandedType;
		if (!isOpen)
			return;
		TrackFX_SetOpen(track, devicePosition, 0);
		TrackFX_Show(track, devicePosition, expandedType);
		return;
	}

	if (std::strcmp(part, "param") == 0)
	{
		Process(path, static_cast<double> (value));
		return;
	}

	const int fx = atoi(part) - 1;

	const char* cmd = SafeGet(path, 1);
	if (std::strcmp(cmd, "bypass") == 0)
	{
		if (devicePosition >= 0)
			TrackFX_SetEnabled(track, fx, value > 0 ? 0 : 1);
		return;
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack2(project, 0, true);
	if (track == nullptr)
		return;

	if (std::strcmp(part, "param") == 0 && std::strcmp(SafeGet(path, 2), "value") == 0)
	{
		const int devicePosition = this->GetDeviceSelection();
		const int paramNo = atoi(SafeGet(path, 1));
		PreventUIRefresh(1);
		TrackFX_SetParamNormalized(track, devicePosition, paramNo, value);
		PreventUIRefresh(-1);
	}
}


/** {@inheritDoc} */
void DeviceProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	ReaProject* project = ReaperUtils::GetProject();
	MediaTrack* track = GetSelectedTrack2(project, 0, true);
	if (track == nullptr)
		return;

	if (std::strcmp(part, "add") == 0)
	{
		const char* deviceName = value.c_str();
		std::string deviceNameStr(deviceName);

		// Remove potential quotes which do not work with the Reaper function
		if (deviceNameStr.size() >= 2 && deviceNameStr.front() == '"' && deviceNameStr.back() == '"') {
			deviceNameStr.erase(0, 1);
			deviceNameStr.pop_back();
		}
		const int position = TrackFX_AddByName(track, deviceNameStr.c_str(), false, -1);
		if (position < 0)
			return;
		const int insert = atoi(SafeGet(path, 1));
		TrackFX_CopyToTrack(track, position, track, insert, true);
	}
}
