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

#include "wdltypes.h"
#include "resource.h"

#include "StringUtils.h"
#include "DrivenByMossSurface.h"
#include "ReaDebug.h"

// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
#ifdef _DEBUG
const bool DEBUG_JAVA{ true };
#else
const bool DEBUG_JAVA{ false };
#endif



REAPER_PLUGIN_HINSTANCE g_hInst;

// The global extension variables required to bridge from C to C++
DrivenByMossSurface *gSurface = nullptr;
JvmManager *jvmManager = nullptr;

/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 */
void processNoArgCPP(JNIEnv *env, jobject object, jstring command)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gSurface->GetOscParser().Process(path);
	env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a string parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 * @param value   The string value
 */
void processStringArgCPP(JNIEnv *env, jobject object, jstring command, jstring value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	const char *val = env->GetStringUTFChars(value, JNI_FALSE);
	if (val == nullptr)
	{
		env->ReleaseStringUTFChars(command, cmd);
		return;
	}
	std::string path(cmd);
	std::string valueString(val);
	gSurface->GetOscParser().Process(path, valueString);
	env->ReleaseStringUTFChars(command, cmd);
	env->ReleaseStringUTFChars(value, val);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with an integer parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 * @param value   The integer value
 */
void processIntArgCPP(JNIEnv *env, jobject object, jstring command, jint value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gSurface->GetOscParser().Process(path, value);
	env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a double parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 * @param value   The double value
 */
void processDoubleArgCPP(JNIEnv *env, jobject object, jstring command, jdouble value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gSurface->GetOscParser().Process(path, value);
	env->ReleaseStringUTFChars(command, cmd);
}

/**
 * Java callback to dump the data model.
 *
 * @param env    The JNI environment
 * @param object The JNI object
 * @param dump   Dump all data if true otherwise only the modified since the last call
 * @return The changed model data
 */
jstring receiveModelDataCPP(JNIEnv *env, jobject object, jboolean dump)
{
	if (env == nullptr || gSurface == nullptr)
		return nullptr;
	std::string result = gSurface->CollectData(dump);
	return env->NewStringUTF(result.c_str());
}


static void createJVM()
{
	if (jvmManager == nullptr)
	{
		jvmManager = new JvmManager(DEBUG_JAVA);
		jvmManager->init((void *)&processNoArgCPP, (void *)&processStringArgCPP, (void *)&processIntArgCPP, (void *)&processDoubleArgCPP, (void *)&receiveModelDataCPP);
	}
}


// Callback function for Reaper to create an instance of the extension
static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
	createJVM();
	// Prevent a second instance and ensure that JVM has successfully started...
	return gSurface == nullptr && jvmManager->isRunning() ? new DrivenByMossSurface() : nullptr;
}


// Processing function for the configuration dialog
static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			if (jvmManager != nullptr)
			{
#ifdef _WIN32
				SetDlgItemText(hwndDlg, IDC_JAVA_HOME, stringToWs(jvmManager->GetJavaHomePath()).c_str());
#else
				SetDlgItemText(hwndDlg, IDC_JAVA_HOME, jvmManager->GetJavaHomePath().c_str());
#endif
			}
		}
		break;

		case WM_COMMAND:
		{
			WORD value = LOWORD(wParam);
			switch (value)
			{
				case IDC_BUTTON_CONFIGURE:
					if (jvmManager != nullptr)
						jvmManager->DisplayWindow();
					break;
			}
			break;
		}
	}
	return 0;
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
	createJVM();
	return CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SURFACEEDIT_DRIVENBYMOSS), parent, dlgProc, (LPARAM)initConfigString);
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
			g_hInst = hInstance;

			// On startup...
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0;

			REAPERAPI_LoadAPI(rec->GetFunc);

			int result = rec->Register("csurf", &drivenbymoss_reg);
			if (!result)
				ReaDebug() << ("Could not instantiate DrivenByMoss surface extension.");

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


#ifndef _WIN32 // MAC resources
#include "swell-dlggen.h"
#include "res.rc_mac_dlg"
#undef BEGIN
#undef END
#endif
