// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_FUNCTIONEXECUTOR_H_
#define _DBM_FUNCTIONEXECUTOR_H_

#include <mutex>
#include <vector>
#include <functional>


/**
 * Executes all registered functions in a thread safe way.
 */
class FunctionExecutor
{
public:
	void AddFunction(std::function<void(void)> f);
	void ExecuteFunctions();

private:
	std::mutex execMutex;
	std::vector<std::function<void(void)>> tasks;
};

#endif /* _DBM_FUNCTIONEXECUTOR_H_ */
