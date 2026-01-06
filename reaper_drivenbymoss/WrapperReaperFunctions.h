// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_WRAPPER_REAPER_FUNCTIONS_H_
#define _DBM_WRAPPER_REAPER_FUNCTIONS_H_

#ifdef _WIN32
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#endif

#include <wdltypes.h>
#include "reaper_plugin_functions.h"
#undef max
#undef min

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif /* _DBM_WRAPPER_REAPER_FUNCTIONS_H_ */
