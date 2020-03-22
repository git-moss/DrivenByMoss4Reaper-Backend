// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.tvaluet

#include "Model.h"
#include "ReaDebug.h"
#include "ReaperUtils.h"
#include "StringUtils.h"

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

		std::ostringstream out;
		out << "drivenbymoss: " << bufferStr << "\n";
		const std::string msg = out.str();

		if (ReaDebug::model == nullptr)
		{
			ShowConsoleMsg(msg.c_str());
			return;
		}

		ReaDebug::model->AddFunction([msg]() noexcept
			{
				ShowConsoleMsg(msg.c_str());
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
	try
	{
#ifdef _WIN32
		OutputDebugString(stringToWs(msg).c_str());
#else
		// TODO
#endif
	}
	catch (...)
	{
		// Nothing we can do about it ...
	}
}


void ReaDebug::Log(const char* msg) noexcept
{
	try
	{
#ifdef _WIN32
		OutputDebugString(stringToWs(msg).c_str());
#else
		// TODO
#endif
	}
	catch (...)
	{
		// Nothing we can do about it ...
	}
}


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