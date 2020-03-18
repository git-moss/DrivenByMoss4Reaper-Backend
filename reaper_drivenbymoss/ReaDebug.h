// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_READEBUG_H_
#define _DBM_READEBUG_H_

#include <string>
#include "Model.h"


/**
 * Collects a message text and dumps it to the Reaper console when desctructed.
 */
class ReaDebug
{
public:
	static void init(Model *aModel) noexcept
	{
		model = aModel;
	}

	ReaDebug() noexcept;
	ReaDebug(const ReaDebug&) = delete;
	ReaDebug& operator=(const ReaDebug&) = delete;
	ReaDebug(ReaDebug&&) = delete;
	ReaDebug& operator=(ReaDebug&&) = delete;
	~ReaDebug();

	ReaDebug &operator << (const char *value);
	ReaDebug &operator << (int value);
	ReaDebug &operator << (size_t value);
	ReaDebug &operator << (int64_t value);
	ReaDebug &operator << (double value);
	ReaDebug &operator << (void *value);
	ReaDebug &operator << (const std::string& value);

private:
	static Model *model;

	std::string buffer;
};

#endif /* _DBM_READEBUG_H_ */