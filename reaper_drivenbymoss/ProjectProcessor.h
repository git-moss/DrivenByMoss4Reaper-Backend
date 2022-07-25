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
	ProjectProcessor(Model &model);

	void Process(std::deque<std::string> &path) override;
	void Process(std::deque<std::string>& path, int value) override;

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};

private:
	void CreateRegion(ReaProject* project, double start, double length) noexcept;
};

#endif /* _DBM_PROJECTPROCESSOR_H_ */