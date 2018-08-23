// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

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
