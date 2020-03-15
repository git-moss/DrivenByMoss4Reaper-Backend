// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "FunctionExecutor.h"


/**
 * Add a function for execution.
 *
 * @param f The function too add for execution
 */
void FunctionExecutor::AddFunction(std::function<void(void)> f)
{
	this->execMutex.lock();
	this->tasks.push_back(f);
	this->execMutex.unlock();
}


/**
 * Execute all registered functions.
 */
void FunctionExecutor::ExecuteFunctions()
{
	this->execMutex.lock();
	for (auto &task : this->tasks)
		task();
	this->tasks.clear();
	this->execMutex.unlock();
}
