// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_READEBUG_H_
#define _DBM_READEBUG_H_

#include <chrono>
#include <iostream>
#include <sstream>

#include "Model.h"

constexpr bool ENABLE_EXTENSION{ true };
constexpr bool ENABLE_JAVA{ true };
constexpr bool ENABLE_JAVA_START{ true };

// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
#ifdef _DEBUG
constexpr bool DEBUG_JAVA{ true };
#else
constexpr bool DEBUG_JAVA{ false };
#endif


/**
 * Collects a message text and dumps it to the Reaper console when desctructed.
 */
class ReaDebug
{
public:
	ReaDebug() noexcept;
	ReaDebug(const ReaDebug&) = delete;
	ReaDebug& operator=(const ReaDebug&) = delete;
	ReaDebug(ReaDebug&&) = delete;
	ReaDebug& operator=(ReaDebug&&) = delete;
	~ReaDebug();

	ReaDebug& operator << (const char* value) noexcept;
	ReaDebug& operator << (int value);
	ReaDebug& operator << (size_t value);
	ReaDebug& operator << (int64_t value);
	ReaDebug& operator << (double value);
	ReaDebug& operator << (void* value);
	ReaDebug& operator << (const std::string& value);

	static void setModel(Model* aModel) noexcept
	{
		model = aModel;
	}

	static void Log(const std::string& msg) noexcept;
	static void Log(const char* msg) noexcept;
	static void Log(const int& value) noexcept;
	static void Log(const double& value) noexcept;

	void Measure() noexcept;

private:
	static Model* model;

	std::ostringstream buffer;
	std::chrono::time_point<std::chrono::steady_clock> start{ std::chrono::steady_clock::now() };
};

#endif /* _DBM_READEBUG_H_ */