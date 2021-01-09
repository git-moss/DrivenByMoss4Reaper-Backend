// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_ACTIONPROCESSOR_H_
#define _DBM_ACTIONPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to actions.
 */
class ActionProcessor : public OscProcessor
{
public:
	ActionProcessor(Model& model) noexcept;

	void Process(std::deque<std::string>& path) noexcept override;
	void Process(std::deque<std::string>& path, int value) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

	void CollectData(std::ostringstream& ss);

	void CheckActionSelection();

private:
	bool selectionIsActive;
	int selectedAction;
};

#endif /* _DBM_ACTIONPROCESSOR_H_ */
