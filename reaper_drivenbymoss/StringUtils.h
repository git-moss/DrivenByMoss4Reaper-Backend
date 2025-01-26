// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_STRINGUTILS_H_
#define _DBM_STRINGUTILS_H_

#include "ReaDebug.h"

std::wstring stringToWs(const std::string& src);

/**
 * Convert a string to a wide string.
 */
std::string wstringToDefaultPlatformEncoding(const std::wstring& src);

// make_string
class MakeString
{
public:
	template <typename T>
	MakeString& operator<<(T const& val)
	{
		buffer << val;
		return *this;
	}

	operator std::string() const
	{
		return buffer.str();
	}

private:
	std::ostringstream buffer;
};

#endif /* _DBM_STRINGUTILS_H_ */
