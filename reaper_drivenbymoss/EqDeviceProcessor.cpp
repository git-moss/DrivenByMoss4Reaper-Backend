// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "EqDeviceProcessor.h"
#include "StringUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model
 */
EqDeviceProcessor::EqDeviceProcessor(Model& aModel) noexcept : DeviceProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
int EqDeviceProcessor::GetDeviceSelection() noexcept
{
	MediaTrack* track = GetSelectedTrack2(ReaperUtils::GetProject(), 0, true);
	if (track == nullptr)
		return -1;
	return TrackFX_GetEQ(track, false);
}


/** {@inheritDoc} */
void EqDeviceProcessor::Process(std::deque<std::string>& path) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	if (std::strcmp(part, "add") == 0)
	{
		MediaTrack* track = GetSelectedTrack2(ReaperUtils::GetProject(), 0, true);
		if (track != nullptr)
			TrackFX_GetEQ(track, true);
		return;
	}

	DeviceProcessor::Process(path);
}


/** {@inheritDoc} */
void EqDeviceProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;
	const char* part = SafeGet(path, 0);

	MediaTrack* track = GetSelectedTrack2(ReaperUtils::GetProject(), 0, true);
	if (track == nullptr)
		return;

	if (std::strcmp(part, "band") == 0)
	{
		const int eqIndex = TrackFX_GetEQ(track, false);
		if (eqIndex < 0)
			return;

		const int bandNo = atoi(SafeGet(path, 1));

		// Off?
		const bool isOff = std::strcmp(value.c_str(), "-1") == 0;
		if (!isOff)
		{
			std::string btss = MakeString() << "BANDTYPE" << bandNo;
			TrackFX_SetNamedConfigParm(track, eqIndex, btss.c_str(), value.c_str());
		}
		std::string bess = MakeString() << "BANDENABLED" << bandNo;
		TrackFX_SetNamedConfigParm(track, eqIndex, bess.c_str(), isOff ? "0" : "1");
		return;
	}
}
