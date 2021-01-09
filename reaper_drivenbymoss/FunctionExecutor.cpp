// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "FunctionExecutor.h"


/**
 * Add a function for execution.
 *
 * @param f The function too add for execution
 */
void FunctionExecutor::AddFunction(std::function<void(void)> f)
{
	const std::lock_guard<std::mutex> lock(this->execMutex);
	this->tasks.push_back(f);
}


/**
 * Execute all registered functions.
 */
void FunctionExecutor::ExecuteFunctions()
{
	const std::lock_guard<std::mutex> lock(this->execMutex);
	for (auto &task : this->tasks)
		task();
	this->tasks.clear();
}
