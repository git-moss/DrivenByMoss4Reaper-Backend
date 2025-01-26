// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "IniFileProcessor.h"
#include "StringUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
IniFileProcessor::IniFileProcessor(Model& aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void IniFileProcessor::Process(std::deque<std::string>& path, int value) noexcept
{
	if (path.size() != 2)
		return;

	try
	{
#ifdef _WIN32
		const std::wstring category = stringToWs(path.at(0));
		const std::string varName = path.at(1);
		const std::wstring key = stringToWs(varName);
		const std::wstring iniPath = stringToWs(GetIniName());
#else
		const std::string category = path.at(0);
		const std::string varName = path.at(1);
		const std::string key = varName;
		const std::string iniPath = GetIniName();
#endif			
		const int currValue = GetPrivateProfileInt(category.c_str(), key.c_str(), -1, iniPath.c_str());
		if (currValue == value)
			return;

#ifdef _WIN32
		const std::wstring v = stringToWs(std::to_string(value));
#else
		const std::string v = std::to_string(value);
#endif
		if (!WritePrivateProfileString(category.c_str(), key.c_str(), v.c_str(), iniPath.c_str()))
			ReaDebug() << "ERROR: Could not store parameter in REAPER.ini";

		// Also write memory but currently only supports 'double' values!
		int size;
		void* addr = get_config_var(varName.c_str(), &size);
		if (addr != nullptr && size == sizeof(double))
		{
			double* m_addr = static_cast<double*>(addr);
			if (m_addr != nullptr)
			{
				double measures = value;
				*m_addr = measures;
			}
		}
	}
	catch (...)
	{
		ReaDebug() << "ERROR: Could not store parameter in REAPER.ini";
	}
}


/**
 * Get the absolute path of the REAPER.ini file.
 *
 * @return The path
 */
const std::string IniFileProcessor::GetIniName() const
{
	std::ostringstream ss;
	ss << GetResourcePath();
#ifdef _WIN32
	ss << "\\";
#else
	ss << "/";
#endif			
	ss << "REAPER.ini";
	return ss.str();
}
