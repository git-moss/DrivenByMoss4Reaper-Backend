// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>


/**
 * Collects a message text and dumps it to the Reaper console when desctructed.
 */
class ReaDebug
{
public:
	ReaDebug() noexcept;
	~ReaDebug();

	ReaDebug &operator << (const char *value);
	ReaDebug &operator << (int value);
	ReaDebug &operator << (size_t value);
	ReaDebug &operator << (int64_t value);
	ReaDebug &operator << (double value);
	ReaDebug &operator << (void *value);
	ReaDebug &operator << (const std::string& value);

private:
	std::string buffer;
};
