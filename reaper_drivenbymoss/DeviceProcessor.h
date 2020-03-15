// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
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
	DeviceProcessor(Model &model);

	void Process(std::deque<std::string> &path) override;
	void Process(std::deque<std::string> &path, int value) override;
	void Process(std::deque<std::string> &path, double value) override;
	void Process(std::deque<std::string> &path, const std::string &value) override;

private:
	void SetDeviceSelection(int position);
};

#endif /* _DBM_DEVICEPROCESSOR_H_ */