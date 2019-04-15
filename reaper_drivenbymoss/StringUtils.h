// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include "ReaDebug.h"

std::wstring stringToWs(const std::string &src);

/**
 * Convert a string to a wide string.
 */
std::string wstringToDefaultPlatformEncoding(const std::wstring &src);