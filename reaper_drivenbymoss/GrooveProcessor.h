// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_GROOVEPROCESSOR_H_
#define _DBM_GROOVEPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to groove.
 */
class GrooveProcessor : public OscProcessor
{
public:
	GrooveProcessor(Model &model) noexcept;

	void Process(std::deque<std::string>& path, double value) noexcept override;
};

#endif /* _DBM_GROOVEPROCESSOR_H_ */