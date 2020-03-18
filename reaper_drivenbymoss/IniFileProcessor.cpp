// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <sstream>

#include "IniFileProcessor.h"
#include "StringUtils.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
IniFileProcessor::IniFileProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void IniFileProcessor::Process(std::deque<std::string>& path, int value) noexcept
{
	if (path.size() != 2)
		return;

#ifdef _WIN32
	const std::wstring category = stringToWs(path.at(0));
	const std::wstring key = stringToWs(path.at(1));
	const std::wstring iniPath = stringToWs(GetIniName());
#else
	const std::string category = path.at(0);
	const std::string key = path.at(1);
	const std::string iniPath = GetIniName();
#endif			

	const int currValue = GetPrivateProfileInt(category.c_str(), key.c_str(), -1, iniPath.c_str());
	if (currValue == value)
		return;

	std::stringstream valStream;
	valStream << value;
#ifdef _WIN32
	const std::wstring v = stringToWs(valStream.str());
#else
	const std::string v = valStream.str();
#endif
	if (!WritePrivateProfileString(category.c_str(), key.c_str(), v.c_str(), iniPath.c_str()))
		ReaDebug() << "ERROR: Could not store parameter in REAPER.ini";
}


/**
 * Get the absolute path of the REAPER.ini file.
 *
 * @return The path
 */
const std::string IniFileProcessor::GetIniName() const
{
	std::stringstream ss;
	ss << GetResourcePath();
#ifdef _WIN32
	ss << "\\";
#else
	ss << "/";
#endif			
	ss << "REAPER.ini";
	return ss.str();
}