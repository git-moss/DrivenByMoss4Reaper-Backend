// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "DrivenByMossSurface.h"
#include "de_mossgrabers_transformator_TransformatorApplication.h"


// Enable or disable for debugging. If debugging is enabled Reaper is waiting for a Java debugger
// to be connected on port 8989, only then the start continues!
const bool DEBUG = false;

// The global extension variables required to bridge from C to C++
static DrivenByMossExtension *gExtension;


/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @param command The command to execute
 */
void processNoArgCPP(JNIEnv *env, jobject object, jstring command)
{
	if (env == nullptr || gExtension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gExtension->GetOscParser().Process(path);
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
	if (env == nullptr || gExtension == nullptr)
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
	gExtension->GetOscParser().Process(path, valueString);
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
	if (env == nullptr || gExtension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gExtension->GetOscParser().Process(path, value);
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
	if (env == nullptr || gExtension == nullptr)
		return;
	const char *cmd = env->GetStringUTFChars(command, JNI_FALSE);
	if (cmd == nullptr)
		return;
	std::string path(cmd);
	gExtension->GetOscParser().Process(path, value);
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
	if (env == nullptr || gExtension == nullptr)
		return nullptr;
	std::string result = gExtension->CollectData(dump);
	return env->NewStringUTF(result.c_str());
}



/**
 * Constructor.
 */
DrivenByMossSurface::DrivenByMossSurface() : extension(functionExecutor, DEBUG)
{
	gExtension = &extension;

	std::string currentPath = GetExePath();
	JvmManager &jvmManager = this->extension.GetJvmManager();
	jvmManager.Create(currentPath);
	jvmManager.RegisterMethods(&processNoArgCPP, &processStringArgCPP, &processIntArgCPP, &processDoubleArgCPP, &receiveModelDataCPP);
	jvmManager.StartApp();
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	gExtension = nullptr;
}
