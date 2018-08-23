// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#ifdef _WIN32
#include <codecvt>


/**
 * Convert a string to a wide string.
 */
std::wstring stringToWs(const std::string& s)
{
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(s);
}

#endif
