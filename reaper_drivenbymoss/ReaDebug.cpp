// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.tvaluet

#include "ReaDebug.h"
#include "reaper_plugin_functions.h"


ReaDebug::ReaDebug() noexcept
{
	buffer.append("drivenbymoss: ");
}

ReaDebug::~ReaDebug()
{
	buffer.append("\n");
	ShowConsoleMsg(buffer.c_str());
}

ReaDebug &ReaDebug::operator << (const char *value)
{
	buffer.append(value);
	return *this;
}

ReaDebug &ReaDebug::operator << (int value)
{
	buffer.append(std::to_string(value));
	return *this;
}

ReaDebug &ReaDebug::operator << (size_t value)
{
	buffer.append(std::to_string(value));
	return *this;
}
ReaDebug &ReaDebug::operator << (int64_t value)
{
	buffer.append(std::to_string(value));
	return *this;
}
ReaDebug &ReaDebug::operator << (double value)
{
	char buf[64];
	sprintf_s(buf, "%.2f", value);
	buffer.append(buf);
	return *this;
}

ReaDebug &ReaDebug::operator << (void *value)
{
	char buf[32];
	sprintf_s(buf, "%p", value);
	buffer.append(buf);
	return *this;
}

ReaDebug &ReaDebug::operator << (const std::string &value)
{
	buffer.append(value);
	return *this;
}
