// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_STRINGUTILS_H_
#define _DBM_STRINGUTILS_H_

#include "ReaDebug.h"

std::wstring stringToWs(const std::string &src);

/**
 * Convert a string to a wide string.
 */
std::string wstringToDefaultPlatformEncoding(const std::wstring &src);

#endif /* _DBM_STRINGUTILS_H_ */