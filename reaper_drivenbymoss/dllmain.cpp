// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
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

#include "DrivenByMossSurface.h"
#include "ReaDebug.h"
#include "StringUtils.h"

// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
#ifdef _DEBUG
const bool DEBUG_JAVA{ true };
#else
const bool DEBUG_JAVA{ false };
#endif


REAPER_PLUGIN_HINSTANCE g_hInst;

gaccel_register_t openDBMConfigureWindowAccel = { {0,0,0}, "DrivenByMoss: Open the configuration window." };

// The global extension variables required to bridge from C to C++
DrivenByMossSurface* gSurface = nullptr;
JvmManager* jvmManager = nullptr;

/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 */
void processNoArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, JNI_FALSE);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, JNI_FALSE);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	gSurface->GetOscParser().Process(procstr, path);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a string parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The string value
 */
void processStringArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jstring value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, JNI_FALSE);
	if (proc == nullptr)
		return;
	const char* val = env->GetStringUTFChars(value, JNI_FALSE);
	if (val == nullptr)
	{
		env->ReleaseStringUTFChars(command, proc);
		return;
	}
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, JNI_FALSE);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	std::string valueString(val);
	gSurface->GetOscParser().Process(procstr, path, valueString);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
	env->ReleaseStringUTFChars(value, val);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with an integer parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The integer value
 */
void processIntArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jint value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, JNI_FALSE);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, JNI_FALSE);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	gSurface->GetOscParser().Process(procstr, path, value);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a double parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The double value
 */
void processDoubleArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jdouble value)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, JNI_FALSE);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, JNI_FALSE);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	gSurface->GetOscParser().Process(procstr, path, value);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback to delay updates for a specific processor. Use to prevent that Reaper sends old
 * values before the latest ones are applied.
 *
 * @param processor The processor to delay.
 */
void delayUpdatesCPP(JNIEnv* env, jobject object, jstring processor)
{
	if (env == nullptr || gSurface == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, JNI_FALSE);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	
	gSurface->GetDataCollector().DelayUpdate(procstr);

	env->ReleaseStringUTFChars(processor, proc);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a double parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param status  MIDI status byte
 * @param data1   MIDI data byte 1
 * @param data2   MIDI data byte 2
 */
void processMidiArgCPP(JNIEnv* env, jobject object, jint status, jint data1, jint data2)
{
	if (env != nullptr && gSurface != nullptr)
		StuffMIDIMessage(0, status, data1, data2);
}


static void createJVM()
{
	if (jvmManager != nullptr)
		return;
	jvmManager = new JvmManager(DEBUG_JAVA);
	jvmManager->init((void*)& processNoArgCPP, (void*)& processStringArgCPP, (void*)& processIntArgCPP, (void*)& processDoubleArgCPP, (void*)& delayUpdatesCPP, (void*)& processMidiArgCPP);
}


// Callback function for Reaper to create an instance of the extension
static IReaperControlSurface* createFunc(const char* type_string, const char* configString, int* errStats)
{
	createJVM();
	// Prevent a second instance and ensure that JVM has successfully started...
	return gSurface == nullptr && jvmManager->isRunning() ? new DrivenByMossSurface() : nullptr;
}

// Callback for custom actions
bool hookCommandProc(int command, int flag)
{
	if (openDBMConfigureWindowAccel.accel.cmd != 0 && openDBMConfigureWindowAccel.accel.cmd == command)
	{
		if (jvmManager != nullptr)
			jvmManager->DisplayWindow();
		return true;
	}
	return false;
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

static HWND configFunc(const char* type_string, HWND parent, const char* initConfigString)
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
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, const reaper_plugin_info_t* rec)
	{
		if (rec)
		{
			g_hInst = hInstance;

			// On startup...
			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0;

			REAPERAPI_LoadAPI(rec->GetFunc);

			// Register extension
			int result = rec->Register("csurf", &drivenbymoss_reg);
			if (!result)
			{
				ReaDebug() << "Could not instantiate DrivenByMoss surface extension.";
				return 0;
			}

			// Register Open Window action
			openDBMConfigureWindowAccel.accel.cmd = rec->Register("command_id", (void*) "DBM_OPEN_WINDOW_ACTION");
			if (!openDBMConfigureWindowAccel.accel.cmd)
			{
				ReaDebug() << "Could not register ID for DrivenByMoss open window action.";
				return 0;
			}
			if (!rec->Register("gaccel", &openDBMConfigureWindowAccel))
			{
				ReaDebug() << "Could not register DrivenByMoss open window action.";
				return 0;
			}
			if (!rec->Register("hookcommand", (void*)hookCommandProc))
			{
				ReaDebug() << "Could not register DrivenByMoss open window action callback.";
				return 0;
			}

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
#include "dllmain.h"
#undef BEGIN
#undef END
#endif
