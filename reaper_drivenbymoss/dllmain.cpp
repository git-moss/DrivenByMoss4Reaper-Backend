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


// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
const bool DEBUG = false;

// The global extension variables required to bridge from C to C++
DrivenByMossSurface *gSurface = nullptr;
JvmManager *jvmManager = nullptr;


static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
	// Prevent a second instance...
	return gSurface == nullptr ? new DrivenByMossSurface(jvmManager) : nullptr;
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

			jvmManager = new JvmManager(DEBUG);

			int result = rec->Register("csurf", &drivenbymoss_reg);
			if (!result)
				ShowConsoleMsg("Could not instantiate DrivenByMoss surface extension.");

			return 1;
		}
		else
		{
			// On shutdown...
			if (jvmManager != nullptr)
				delete jvmManager;
			return 0;
		}
	}
};