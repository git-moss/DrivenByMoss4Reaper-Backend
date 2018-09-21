// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.tvaluet

#include "ReaDebug.h"
#include "ReaperUtils.h"

Model *ReaDebug::model = nullptr;


ReaDebug::ReaDebug() noexcept
{
	buffer.append("drivenbymoss: ");
}


ReaDebug::~ReaDebug()
{
	buffer.append("\n");
	
	if (ReaDebug::model == nullptr)
		ShowConsoleMsg(buffer.c_str());
	else
	{
		const std::string msg = buffer;
		ReaDebug::model->AddFunction([=]()
		{
			ShowConsoleMsg(msg.c_str());
		});
	}
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
#ifdef _WIN32
	sprintf_s(buf, "%.2f", value);
#else
    snprintf(buf, 64, "%.2f", value);
#endif
    buffer.append(buf);
	return *this;
}

ReaDebug &ReaDebug::operator << (void *value)
{
	char buf[32];
#ifdef _WIN32
	sprintf_s(buf, "%p", value);
#else
    snprintf(buf, 32, "%p", value);
#endif
	buffer.append(buf);
	return *this;
}

ReaDebug &ReaDebug::operator << (const std::string &value)
{
	buffer.append(value);
	return *this;
}
