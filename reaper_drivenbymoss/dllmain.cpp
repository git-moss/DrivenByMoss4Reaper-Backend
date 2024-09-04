// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "stdafx.h"

#define REAPERAPI_IMPLEMENT

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>

#include "wdltypes.h"
#include "lineparse.h"
#include "resource.h"

#include "CodeAnalysis.h"
#include "DrivenByMossSurface.h"
#include "ReaDebug.h"
#include "StringUtils.h"

// The global extension variables required to bridge from C to C++,
// static keyword restricts the visibility of a function to the file
REAPER_PLUGIN_HINSTANCE pluginInstanceHandle = nullptr;
gaccel_register_t openDBMConfigureWindowAccel = { {0,0,0}, "DrivenByMoss: Open the configuration window." };
gaccel_register_t openDBMProjectWindowAccel = { {0,0,0}, "DrivenByMoss: Open the project settings window." };
gaccel_register_t openDBMParameterWindowAccel = { {0,0,0}, "DrivenByMoss: Open the parameter settings window." };
gaccel_register_t restartControllersAccel = { {0,0,0}, "DrivenByMoss: Restart all controllers." };
std::unique_ptr <JvmManager> jvmManager;

// Defined in DrivenByMossSurface.cpp
extern DrivenByMossSurface* surfaceInstance;


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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path);
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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* val = env->GetStringUTFChars(value, nullptr);
	if (val == nullptr)
	{
		env->ReleaseStringUTFChars(processor, proc);
		return;
	}
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	std::string valueString(val);
	surfaceInstance->GetOscParser().Process(procstr, path, valueString);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
	env->ReleaseStringUTFChars(value, val);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with several string parameters.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param values    The string values
 */
void processStringArgsCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jobjectArray values)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;

	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;

	const int stringCount = env->GetArrayLength(values);
	std::vector<jstring> paramsSource;
	std::vector<std::string> params;
	for (int i = 0; i < stringCount; i++)
	{
		jobject objectValue = env->GetObjectArrayElement(values, i);
		// jobject is not polymorphic
		DISABLE_WARNING_NO_STATIC_DOWNCAST
			jstring value = static_cast<jstring>(objectValue);
		const char* val = env->GetStringUTFChars(value, nullptr);
		if (val == nullptr)
			continue;
		paramsSource.push_back(value);
		params.push_back(val);

		env->ReleaseStringUTFChars(value, val);
	}

	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);

	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);

	surfaceInstance->GetOscParser().Process(procstr, path, params);

	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path, gsl::narrow_cast<int> (value));
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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path, value);
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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surfaceInstance->GetDataCollector().EnableUpdate(procstr, enable);
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
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	// Nullcheck above is not picked up
	DISABLE_WARNING_DANGLING_POINTER
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surfaceInstance->GetDataCollector().DelayUpdate(procstr);
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
	if (env != nullptr && surfaceInstance != nullptr)
		StuffMIDIMessage(0, status, data1, data2);
}


// Callback for custom actions
bool hookCommandProc(int command, int flag)
{
	if (!jvmManager || !jvmManager->IsRunning())
		return false;

	if (openDBMConfigureWindowAccel.accel.cmd != 0 && openDBMConfigureWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayWindow();
		return true;
	}
	if (openDBMProjectWindowAccel.accel.cmd != 0 && openDBMProjectWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayProjectWindow();
		return true;
	}
	if (openDBMParameterWindowAccel.accel.cmd != 0 && openDBMParameterWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayParameterWindow();
		return true;
	}
	if (restartControllersAccel.accel.cmd != 0 && restartControllersAccel.accel.cmd == command)
	{
		jvmManager->RestartControllers();
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
		std::string path = jvmManager ? jvmManager->GetJavaHomePath() : "Filled when the dialog is opened again...";
#ifdef _WIN32
		SetDlgItemText(hwndDlg, IDC_JAVA_HOME, stringToWs(path).c_str());
#else
		SetDlgItemText(hwndDlg, IDC_JAVA_HOME, path.c_str());
#endif
		if (jvmManager)
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_REOPEN_INFO), SW_HIDE);
		}
		else
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_JAVA_HOME_LBL), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_JAVA_HOME), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_CONFIGURE), SW_HIDE);
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
			if (jvmManager && jvmManager->IsRunning())
				jvmManager->DisplayWindow();
			break;
		default:
			// Ignore the rest
			break;
		}
	}
	break;

	default:
		// Ignore the rest
		break;
	}
	return 0;
}


// Callback function for Reaper to create an instance of the extension
IReaperControlSurface* createFunc(const char* type_string, const char* configString, int* errStats)
{
	if (!ENABLE_EXTENSION)
		return nullptr;

	// Note: If the setup dialog is closed with OK, the current surfaceInstance will be destructed but
	// we cannot create a new JVM, since this is only possible once!
	if (ENABLE_JAVA && !jvmManager)
	{
		jvmManager = std::make_unique<JvmManager>(DEBUG_JAVA);
		if (!jvmManager)
			return nullptr;

		DISABLE_WARNING_PUSH
		DISABLE_WARNING_REINTERPRET_CAST
		void* processNoArgPtr = reinterpret_cast<void*>(&processNoArgCPP);
		void* processStringArgPtr = reinterpret_cast<void*>(&processStringArgCPP);
		void* processStringArgsPtr = reinterpret_cast<void*>(&processStringArgsCPP);
		void* processIntArgPtr = reinterpret_cast<void*>(&processIntArgCPP);
		void* processDoubleArgPtr = reinterpret_cast<void*>(&processDoubleArgCPP);
		void* enableUpdatesPtr = reinterpret_cast<void*>(&enableUpdatesCPP);
		void* delayUpdatesPtr = reinterpret_cast<void*>(&delayUpdatesCPP);
		void* processMidiArgPtr = reinterpret_cast<void*>(&processMidiArgCPP);
		DISABLE_WARNING_POP

		// Nullcheck above is not picked up
		DISABLE_WARNING_PUSH
		DISABLE_WARNING_DANGLING_POINTER
		jvmManager->init(processNoArgPtr, processStringArgPtr, processStringArgsPtr, processIntArgPtr, processDoubleArgPtr, enableUpdatesPtr, delayUpdatesPtr, processMidiArgPtr);
		DISABLE_WARNING_POP
	}

	ReaDebug::Log("DrivenByMoss: Creating surface.\n");

	// Note: delete is called from Reaper on shutdown, no need to do it ourselves
	DISABLE_WARNING_DONT_USE_NEW
	surfaceInstance = new DrivenByMossSurface(jvmManager);

	ReaDebug::Log("DrivenByMoss: Surface created.\n");

	return surfaceInstance;
}

// Callback function for Reaper to create the configuration dialog of the extension
static HWND configFunc(const char* type_string, HWND parent, const char* initConfigString) noexcept
{
	// No way to prevent the LPARAM cast
	DISABLE_WARNING_REINTERPRET_CAST
	return CreateDialogParam(pluginInstanceHandle, MAKEINTRESOURCE(IDD_SURFACEEDIT_DRIVENBYMOSS), parent, dlgProc, reinterpret_cast<LPARAM>(initConfigString));
}

// Description for DrivenByMoss surfaceInstance extension
static reaper_csurf_reg_t drivenbymoss_reg =
{
	"DrivenByMoss4Reaper",
	"DrivenByMoss4Reaper",
	createFunc,
	configFunc,
};


/**
 * Called for all extension sub-tags of the project file.
 */
bool ProcessExtensionLine(const char* line, ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Processing project parameters.\n");

	if (ctx == nullptr)
		return false;

	if (!jvmManager || !jvmManager->IsRunning())
		return false;

	// Parse the line and check if it is valid and belongs to this extension
	LineParser lp(false);
	if (lp.parse(line) || lp.getnumtokens() < 1 || strcmp(lp.gettoken_str(0), "<DRIVEN_BY_MOSS") != 0)
	{
		ReaDebug::Log("DrivenByMoss: Processing project parameters done - none found.\n");
		return false;
	}

	constexpr int LENGTH = 8000;
	std::string data(LENGTH, 0);
	char* dataPointer = &*data.begin();
	while (true)
	{
		if (ctx->GetLine(dataPointer, LENGTH) || lp.parse(dataPointer))
			break;

		const char* token = lp.gettoken_str(0);
		if (token != nullptr)
		{
			std::string tokenStr = token;
			if (tokenStr.at(0) == '>')
				break;
		}

		jvmManager->SetFormattedDocumentSettings(data);
	}

	ReaDebug::Log("DrivenByMoss: Processing project parameters done.\n");
	return true;
}

void SaveExtensionConfig(ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Saving project settings.\n");

	if (ctx == nullptr)
		return;

	if (!jvmManager || !jvmManager->IsRunning())
		return;

	std::string line = jvmManager->GetFormattedDocumentSettings();

	ctx->AddLine("<DRIVEN_BY_MOSS");
	ctx->AddLine("%s", line.c_str());
	ctx->AddLine(">");

	ReaDebug::Log("DrivenByMoss: Project settings saved.\n");
}

void BeginLoadProjectState(bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Loading project settings.\n");

	// Called on project load/undo before any (possible) ProcessExtensionLine. NULL is OK too
	// also called on "new project" (wont be followed by ProcessExtensionLine calls in that case)
	// Defaults could be set here but are already set by the controller instances
	if (jvmManager && jvmManager->IsRunning())
		jvmManager->SetDefaultDocumentSettings();

	ReaDebug::Log("DrivenByMoss: Project settings loaded.\n");
}

project_config_extension_t pcreg =
{
  ProcessExtensionLine,
  SaveExtensionConfig,
  BeginLoadProjectState,
  nullptr,
};


// Must be extern to be exported from the DLL
extern "C"
{
	// Defines the entry point for the DLL application.
	// It is always executed after Reaper loaded it, even if the extension is not added in the configuration dialog!
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, const reaper_plugin_info_t* rec)
	{
		if (!ENABLE_EXTENSION)
			return 0;

		// On shutdown
		if (rec == nullptr)
		{
			if (jvmManager)
			{
				jvmManager->SetCleanShutdown();
				jvmManager.reset();
			}
			return 0;
		}

		// On startup...

		pluginInstanceHandle = hInstance;
		ReaperUtils::mainWindowHandle = rec->hwnd_main;

		if (rec->caller_version != REAPER_PLUGIN_VERSION || rec->GetFunc == nullptr)
			return 0;

		// False positive, null check above is not detected
		DISABLE_WARNING_DANGLING_POINTER
		REAPERAPI_LoadAPI(rec->GetFunc);

		// Register extension
		const int result = rec->Register("csurf", &drivenbymoss_reg);
		if (!result)
		{
			ReaDebug() << "Could not instantiate DrivenByMoss surfaceInstance extension.";
			return 0;
		}

		// Register project notifications
		rec->Register("projectconfig", &pcreg);

		// Register actions
		openDBMConfigureWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_WINDOW_ACTION");
		if (!openDBMConfigureWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open configuration window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMConfigureWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open window action.";
			return 0;
		}

		openDBMProjectWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_PROJECT_WINDOW_ACTION");
		if (!openDBMProjectWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open project window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMProjectWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open project window action.";
			return 0;
		}

		openDBMParameterWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_PARAMETER_WINDOW_ACTION");
		if (!openDBMParameterWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open parameter mapping window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMParameterWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open parameter window action.";
			return 0;
		}

		restartControllersAccel.accel.cmd = rec->Register("command_id", (void*)"RESTART_CONTROLLERS_ACTION");
		if (!restartControllersAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss restart controllers action.";
			return 0;
		}
		if (!rec->Register("gaccel", &restartControllersAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss restart controllers action.";
			return 0;
		}

		if (!rec->Register("hookcommand", (void*)hookCommandProc))
		{
			ReaDebug() << "Could not register DrivenByMoss action callback.";
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
