// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "stdafx.h"

#define REAPERAPI_IMPLEMENT

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>

#include "wdltypes.h"
#include "resource.h"

#include "CodeAnalysis.h"
#include "DrivenByMossSurface.h"
#include "ReaDebug.h"
#include "StringUtils.h"

// The global extension variables required to bridge from C to C++,
// static keyword restricts the visibility of a function to the file
REAPER_PLUGIN_HINSTANCE pluginInstanceHandle;
gaccel_register_t openDBMConfigureWindowAccel = { {0,0,0}, "DrivenByMoss: Open the configuration window." };
DrivenByMossSurface* surface;
std::unique_ptr <JvmManager> jvmManager;


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
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surface->GetOscParser().Process(procstr, path);
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
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* val = env->GetStringUTFChars(value, nullptr);
	if (val == nullptr)
	{
		env->ReleaseStringUTFChars(command, proc);
		return;
	}
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	std::string valueString(val);
	surface->GetOscParser().Process(procstr, path, valueString);
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
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surface->GetOscParser().Process(procstr, path, value);
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
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surface->GetOscParser().Process(procstr, path, value);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback to dis-/enable updates for a specific processor.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to delay
 * @param enable    True to enable updates for the processor
 */
void enableUpdatesCPP(JNIEnv* env, jobject object, jstring processor, jboolean enable)
{
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surface->GetDataCollector().EnableUpdate(procstr, enable);
	env->ReleaseStringUTFChars(processor, proc);
}


/**
 * Java callback to delay updates for a specific processor. Use to prevent that Reaper sends old
 * values before the latest ones are applied.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to delay
 */
void delayUpdatesCPP(JNIEnv* env, jobject object, jstring processor)
{
	if (env == nullptr || surface == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
		const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surface->GetDataCollector().DelayUpdate(procstr);
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
void processMidiArgCPP(const JNIEnv* env, jobject object, jint status, jint data1, jint data2) noexcept
{
	if (env != nullptr && surface != nullptr)
		StuffMIDIMessage(0, status, data1, data2);
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
		DISABLE_WARNING_NO_C_STYLE_CONVERSION
			const WORD value = LOWORD(wParam);
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


// Callback function for Reaper to create an instance of the extension
IReaperControlSurface* createFunc(const char* type_string, const char* configString, int* errStats)
{
	if (!ENABLE_EXTENSION)
		return nullptr;

	// Note: If the setup dialog is closed with OK, the current surface will be destructed but
	// we cannot creat a new JVM, since this is only possible once! Furthermore, we cannot distinct
	// between this and a real shutdown
	if (surface != nullptr)
		return surface->isShutdown ? nullptr : surface;

	if (ENABLE_JAVA)
	{
		jvmManager = std::make_unique<JvmManager>(DEBUG_JAVA);
		if (jvmManager.get() == nullptr)
			return nullptr;

		DISABLE_WARNING_PUSH
		DISABLE_WARNING_REINTERPRET_CAST
		void* processNoArgPtr = reinterpret_cast<void*>(&processNoArgCPP);
		void* processStringArgPtr = reinterpret_cast<void*>(&processStringArgCPP);
		void* processIntArgPtr = reinterpret_cast<void*>(&processIntArgCPP);
		void* processDoubleArgPtr = reinterpret_cast<void*>(&processDoubleArgCPP);
		void* enableUpdatesPtr = reinterpret_cast<void*>(&enableUpdatesCPP);
		void* delayUpdatesPtr = reinterpret_cast<void*>(&delayUpdatesCPP);
		void* processMidiArgPtr = reinterpret_cast<void*>(&processMidiArgCPP);
		DISABLE_WARNING_POP

		// Nullcheck above is not picked up
		DISABLE_WARNING_PUSH
		DISABLE_WARNING_DANGLING_POINTER
		jvmManager->init(processNoArgPtr, processStringArgPtr, processIntArgPtr, processDoubleArgPtr, enableUpdatesPtr, delayUpdatesPtr, processMidiArgPtr);
		DISABLE_WARNING_POP
	}

	// Note: delete is called from Reaper on shutdown, no need to do it ourselves
	DISABLE_WARNING_DONT_USE_NEW
	surface = new DrivenByMossSurface(jvmManager);
	return surface;
}

// Callback function for Reaper to create the configuration dialog of the extension
static HWND configFunc(const char* type_string, HWND parent, const char* initConfigString) noexcept
{
	// No way to prevent the LPARAM cast
	DISABLE_WARNING_REINTERPRET_CAST
		return CreateDialogParam(pluginInstanceHandle, MAKEINTRESOURCE(IDD_SURFACEEDIT_DRIVENBYMOSS), parent, dlgProc, reinterpret_cast<LPARAM>(initConfigString));
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
		if (rec == nullptr || !ENABLE_EXTENSION)
			return 0;

		// On startup...

		pluginInstanceHandle = hInstance;

		if (rec->caller_version != REAPER_PLUGIN_VERSION || rec->GetFunc == nullptr)
			return 0;

		// False positive, null check above is not detected
		DISABLE_WARNING_DANGLING_POINTER
		REAPERAPI_LoadAPI(rec->GetFunc);

		// Register extension
		const int result = rec->Register("csurf", &drivenbymoss_reg);
		if (!result)
		{
			ReaDebug() << "Could not instantiate DrivenByMoss surface extension.";
			return 0;
		}

		// Register Open Window action
		openDBMConfigureWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_WINDOW_ACTION");
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
};


#ifndef _WIN32 // MAC resources
#include "swell-dlggen.h"
#include "res.rc_mac_dlg"
#include "dllmain.h"
#undef BEGIN
#undef END
#endif
