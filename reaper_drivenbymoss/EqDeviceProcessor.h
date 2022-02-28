// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_EQDEVICEPROCESSOR_H_
#define _DBM_EQDEVICEPROCESSOR_H_

#include "DeviceProcessor.h"

/**
 * Processes all commands related to equalizer devices.
 */
class EqDeviceProcessor : public DeviceProcessor
{
public:
	EqDeviceProcessor(Model& model) noexcept;

	void Process(std::deque<std::string>& path) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

protected:
	virtual int GetDeviceSelection() noexcept override;
};

#endif /* _DBM_DEVICEPROCESSOR_H_ */