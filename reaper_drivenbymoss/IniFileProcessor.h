// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_INIFILEPROCESSOR_H_
#define _DBM_INIFILEPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Interface to processing commands and executing them on Reaper related to INI files.
 */
class IniFileProcessor : public OscProcessor
{
public:
	IniFileProcessor(Model& aModel) noexcept;

	void Process(std::deque<std::string>& path, int value) noexcept override;

private:
	const std::string GetIniName() const;
};

#endif /* _DBM_INIFILEPROCESSOR_H_ */