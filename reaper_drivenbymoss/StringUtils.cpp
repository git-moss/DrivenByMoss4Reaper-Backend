// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "WrapperGSL.h"
#include "StringUtils.h"

#ifndef _WIN32
#include <locale>
#include <codecvt>
#include "ModdedCodecvt.h"
#endif

std::wstring stringToWs(const std::string &src)
{
#ifdef _WIN32
	const int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
	std::wstring dest(sizeNeeded, 0);
	DISABLE_WARNING_USE_GSL_AT
	MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, &dest[0], sizeNeeded);
	return dest;
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.from_bytes(src);
#endif
}

/**
 * Convert a string to a wide string.
 */
std::string wstringToDefaultPlatformEncoding(const std::wstring &src)
{
#ifdef _WIN32
	// Remove 1 from the size since std::string does not need null-termination
	const int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL) - 1;
	std::string dest(sizeNeeded, 0);
	WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &dest[0], sizeNeeded, NULL, NULL);
	return dest;
#else
	try
	{
		std::wstring_convert<codecvt<wchar_t, char, mbstate_t>, wchar_t> converter;
		return converter.to_bytes(src);
	}
	catch (const std::range_error & exception)
	{
		// Thrown for bad conversions
		ReaDebug() << "Could not convert wstring to platform wide characters: " << exception.what();
		return "";
	}
#endif
}
