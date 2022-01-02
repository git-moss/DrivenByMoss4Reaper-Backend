// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_PROJECTPROCESSOR_H_
#define _DBM_PROJECTPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to projects.
 */
class ProjectProcessor : public OscProcessor
{
public:
	ProjectProcessor(Model &model) noexcept;

	void Process(std::deque<std::string> &path) noexcept override;
	void Process(std::deque<std::string>& path, int value) noexcept override;

private:
	void CreateRegion(ReaProject* project, double start, double length) noexcept;
};

#endif /* _DBM_PROJECTPROCESSOR_H_ */