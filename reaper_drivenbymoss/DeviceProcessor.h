// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_DEVICEPROCESSOR_H_
#define _DBM_DEVICEPROCESSOR_H_

#include "OscProcessor.h"
#include "Model.h"


/**
 * Processes all commands related to devices.
 */
class DeviceProcessor : public OscProcessor
{
public:
	DeviceProcessor(Model& model);

	void Process(std::deque<std::string>& path) noexcept override;
	void Process(std::deque<std::string>& path, int value) noexcept override;
	void Process(std::deque<std::string>& path, double value) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};

protected:
	virtual int GetDeviceSelection() noexcept;

private:
	void SetDeviceSelection(int position) noexcept;
};

#endif /* _DBM_DEVICEPROCESSOR_H_ */
