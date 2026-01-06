// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_ACTIONPROCESSOR_H_
#define _DBM_ACTIONPROCESSOR_H_

#include "OscProcessor.h"


/**
 * Processes all commands related to actions.
 */
class ActionProcessor : public OscProcessor
{
public:
	ActionProcessor(Model& aModel);

	void Process(std::deque<std::string>& path) noexcept override;
	void Process(std::deque<std::string>& path, int value) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};

	void CollectData(std::ostringstream& ss);

	void CheckActionSelection() noexcept;

private:
	bool selectionIsActive;
	int selectedAction;
};

#endif /* _DBM_ACTIONPROCESSOR_H_ */
