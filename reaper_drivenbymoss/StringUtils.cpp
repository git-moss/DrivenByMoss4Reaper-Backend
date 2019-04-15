// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "StringUtils.h"

#ifndef _WIN32
#include <locale>
#include <codecvt>
#endif

std::wstring stringToWs(const std::string &src)
{
#ifdef _WIN32
	int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
	std::wstring dest(sizeNeeded, 0);
	MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, &dest[0], sizeNeeded);
	return dest;
#else
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	return converter.from_bytes(s);
#endif
}

/**
 * Convert a string to a wide string.
 */
std::string wstringToDefaultPlatformEncoding(const std::wstring &src)
{
#ifdef _WIN32
	// Remove 1 from the size since std::string does not need null-termination
	int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL) - 1;
	std::string dest(sizeNeeded, 0);
	WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &dest[0], sizeNeeded, NULL, NULL);
	return dest;
#else
	try
	{
		std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>, wchar_t> converter;
		return converter.to_bytes(s);
	}
	catch (const std::range_error & exception)
	{
		// Thrown for bad conversions
		ReaDebug() << "Could not convert wstring to platform wide characters: " << exception.what() << " String: " << s;
		return "";
	}
#endif
}
