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

#include "reaper_plugin_functions.h"
#include "OscParser.h"
#include "de_mossgrabers_transformator_TransformatorApplication.h"
#include "JvmManager.h"
#include "DataCollector.h"

// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
const bool DEBUG = true;

// Some global variables required to map from C to C++
REAPER_PLUGIN_HINSTANCE g_hInst;
reaper_plugin_info_t* g_plugin_info;
HWND g_parent;
OscParser *oscParser = nullptr;
DataCollector *dataCollector = nullptr;
JvmManager *jvmManager = nullptr;
Model *model = nullptr;


/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 */
void processNoArgCPP(JNIEnv *env, jobject object, jstring command)
{
	if (oscParser == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, 0);
	std::string path(cmd);
	oscParser->Process(path);
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
	if (oscParser == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, 0);
	const char *val = env->GetStringUTFChars(value, 0);
	std::string path(cmd);
	oscParser->Process(path, val);
	env->ReleaseStringUTFChars(command, cmd);
	env->ReleaseStringUTFChars(command, val);
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
	if (oscParser == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, 0);
	std::string path(cmd);
	oscParser->Process(path, value);
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
	if (oscParser == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, 0);
	std::string path(cmd);
	oscParser->Process(path, value);
	env->ReleaseStringUTFChars(command, cmd);
}

/**
 * Java callback to dump the data model.
 *
 * @param env    The JNI environment
 * @param object The JNI object
 * @param dump   Dump all data if true otherwise only the modified since the last call
 */
jstring receiveModelDataCPP(JNIEnv *env, jobject object, jboolean dump)
{
	std::string result = dataCollector->CollectData(dump);
	return env->NewStringUTF(result.c_str());
}


// Must be extern to be exported from the DLL
extern "C"
{
	// Defines the entry point for the DLL application.
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
	{
		if (rec)
		{
			// On startup...

			if (rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
				return 0;

			g_hInst = hInstance;
			g_plugin_info = rec;
			g_parent = rec->hwnd_main;

			REAPERAPI_LoadAPI(rec->GetFunc);

			model = new Model();
			oscParser = new OscParser(model);
			dataCollector = new DataCollector(model);

			// Set parameter to true for debugging, note that the JVM will halt 
			// until the Java debugger is connected
			jvmManager = new JvmManager(DEBUG);
			std::string currentPath = GetExePath();
			jvmManager->Create(currentPath);
			jvmManager->RegisterMethods(&processNoArgCPP, &processStringArgCPP, &processIntArgCPP, &processDoubleArgCPP, &receiveModelDataCPP);
			jvmManager->StartApp();

			return 1;
		}
		else
		{
			// On shutdown...
			if (jvmManager != nullptr)
				delete jvmManager;
			if (dataCollector != nullptr)
				delete dataCollector;
			if (oscParser != nullptr)
				delete oscParser;
			if (model != nullptr)
				delete model;
			return 0;
		}
	}
};