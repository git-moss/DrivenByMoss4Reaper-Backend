// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.tvaluet

#include "Model.h"
#include "ReaDebug.h"
#include "ReaperUtils.h"
#include "StringUtils.h"

#ifdef __APPLE__
#include <sys/syslog.h>
#endif

// Initialise the static variable
Model* ReaDebug::model = nullptr;

constexpr const char* DISPLAY_ERROR = "Could not display error message in Reaper.";


/**
 * Constructor.
 */
ReaDebug::ReaDebug() noexcept
{
	// Intentionally empty
}


/**
 * Destructor.
 */
ReaDebug::~ReaDebug()
{
	try
	{
		const std::string bufferStr = buffer.str();
		if (bufferStr.empty())
			return;

		DISABLE_WARNING_CAN_THROW
		std::ostringstream out;
		out << "DrivenByMoss: " << bufferStr << "\n";
		const std::string msg = out.str();
        const char *info = msg.c_str();
        ReaDebug::Log(info);
        
		if (ReaDebug::model == nullptr)
		{
			ShowConsoleMsg(info);
			return;
		}

		ReaDebug::model->AddFunction([infoStr = std::string(info)]() noexcept
			{
				ShowConsoleMsg(infoStr.c_str());
			});
	}
	catch (...)
	{
		Log(DISPLAY_ERROR);
	}
}


ReaDebug& ReaDebug::operator << (const char* value) noexcept
{
	try
	{
		this->buffer << value;
	}
	catch (...)
	{
		// Ignore
	}
	return *this;
}


ReaDebug& ReaDebug::operator << (int value)
{
	buffer << value;
	return *this;
}


ReaDebug& ReaDebug::operator << (size_t value)
{
	buffer << value;
	return *this;
}


ReaDebug& ReaDebug::operator << (int64_t value)
{
	buffer << value;
	return *this;
}


ReaDebug& ReaDebug::operator << (double value)
{
	buffer << value;
	return *this;
}


ReaDebug& ReaDebug::operator << (void* value)
{
	buffer << value;
	return *this;
}


ReaDebug& ReaDebug::operator << (const std::string& value)
{
	buffer << value;
	return *this;
}


void ReaDebug::Log(const std::string& msg) noexcept
{
    Log (msg.c_str());
}


void ReaDebug::Log(const char* msg) noexcept
{
	try
	{
#ifdef _WIN32
		OutputDebugString(stringToWs(msg).c_str());
#elif defined __APPLE__
        openlog("DrivenByMoss", (LOG_CONS|LOG_PERROR|LOG_PID), LOG_USER);
        syslog(LOG_NOTICE, "%s", msg);
        closelog();
#else
		std::cout << msg;
#endif
	}
	catch (...)
	{
		// Nothing we can do about it ...
	}
}


void ReaDebug::Log(const int& value) noexcept
{
	Log(std::to_string(value));
}


void ReaDebug::Log(const double& value) noexcept
{
	Log(std::to_string(value));
}


/**
 * Print out the time difference in milliseconds since the last call of the function.
 */
void ReaDebug::Measure() noexcept
{
	try
	{
		const auto end = std::chrono::steady_clock::now();
		const auto diff = end - start;

		std::ostringstream stringStream;
		stringStream << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
		Log(stringStream.str());

		start = end;
	}
	catch (...)
	{
		Log("Crash in measure.");
	}
}
