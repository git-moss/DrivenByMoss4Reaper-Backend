// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "stdafx.h"

#define REAPERAPI_IMPLEMENT

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <jni.h>

#include "DrivenByMossSurface.h"


static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
	return new DrivenByMossSurface();
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
	return NULL;
}


// Description for DrivenByMoss surface extension
static reaper_csurf_reg_t drivenbymoss_reg =
{
	"DrivenByMoss4Reaper",
	"DrivenByMoss4Reaper",
	createFunc,
	configFunc,
};


// Must be extern to be exported from the DLL
extern "C"
{
	// Defines the entry point for the DLL application.
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, const reaper_plugin_info_t *rec)
	{
		if (rec)
		{
			// On startup...
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0;

			REAPERAPI_LoadAPI(rec->GetFunc);

			int result = rec->Register("csurf", &drivenbymoss_reg);
			if (!result)
				ShowConsoleMsg("Could not instantiate DrivenByMoss surface extension.");

			return 1;
		}
		else
		{
			// On shutdown...
			return 0;
		}
	}
};