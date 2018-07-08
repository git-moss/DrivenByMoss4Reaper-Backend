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
#include "de_mossgrabers_transformator_TransformatorApplication.h"
#include "DrivenByMossExtension.h"

// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
const bool DEBUG = false;

// The global extension variables required to bridge from C to C++
std::unique_ptr<DrivenByMossExtension> extension;


/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 */
void processNoArgCPP(JNIEnv *env, jobject object, jstring command)
{
	if (env == nullptr || extension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	extension->GetOscParser().Process(path);
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
	if (env == nullptr || extension == nullptr)
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
	extension->GetOscParser().Process(path, valueString);
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
	if (env == nullptr || extension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	extension->GetOscParser().Process(path, value);
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
	if (env == nullptr || extension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	extension->GetOscParser().Process(path, value);
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
	if (env == nullptr || extension == nullptr)
		return env->NewStringUTF("");
	std::string result = extension->CollectData(dump);
	return env->NewStringUTF(result.c_str());
}


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

			// Set parameter to true for debugging, note that the JVM will halt 
			// until the Java debugger is connected
			extension = std::make_unique<DrivenByMossExtension> (DEBUG);

			std::string currentPath = GetExePath();
			JvmManager &jvmManager = extension->GetJvmManager();
			jvmManager.Create(currentPath);
			jvmManager.RegisterMethods(&processNoArgCPP, &processStringArgCPP, &processIntArgCPP, &processDoubleArgCPP, &receiveModelDataCPP);
			jvmManager.StartApp();

			return 1;
		}
		else
		{
			// On shutdown...
			extension.reset();
			return 0;
		}
	}
};