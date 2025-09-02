// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <codecvt>

#include "CodeAnalysis.h"
#include "JvmManager.h"
#include "ReaDebug.h"
#include "ReaperUtils.h"
#include "StringUtils.h"

#define CURRENT_JNI_VERSION  JNI_VERSION_21


/**
 * Constructor.
 *
 * @param enableDebug True to enable debugging
 */
JvmManager::JvmManager(bool enableDebug) : jvmCmdOptions("-agentlib:jdwp=transport=dt_socket,address=8989,server=y,suspend=y"), jvm(nullptr), jvmLibHandle(nullptr)
{
	this->debug = enableDebug;
	this->options = std::make_unique<JavaVMOption[]>(this->debug ? 2 : 1);
}


/**
 * Destructor.
 */
JvmManager::~JvmManager()
{
	if (this->jvm != nullptr)
	{
        ReaDebug::Log("DrivenByMoss: Shutting down JVM.\n");

		if (this->isCleanShutdown)
		{
            ReaDebug::Log("DrivenByMoss: Shutting down JVM cleanly.\n");

			try
			{
				JNIEnv* env = this->GetEnv();
				if (env != nullptr && this->methodIDShutdown != nullptr)
				{
					env->CallStaticVoidMethod(this->controllerClass, this->methodIDShutdown);
					this->HandleException(*env, "Could not call shutdown.");
				}
			}
			catch (...)
			{
				ReaDebug::Log("Could not call shutdown.\n");
			}
		}

		this->jvm = nullptr;
	}

	ReaDebug::Log("DrivenByMoss: Release JVM library resources.\n");

	this->options.reset();

	// Unload JVM library
	if (this->jvmLibHandle && this->isCleanShutdown)
	{
#ifdef _WIN32
		FreeLibrary(this->jvmLibHandle);
#else
		dlclose(this->jvmLibHandle);
#endif
	}
	this->jvmLibHandle = nullptr;
}


/**
 * Start and initialise the JVM.
 *
 * @param functions The C++ functions to register with JNI
 */
DISABLE_WARNING_ARRAY_POINTER_DECAY
void JvmManager::Init(void* functions[])
{
	if (this->isInitialised)
		return;

    ReaDebug::Log("DrivenByMoss: Creating JVM.\n");
    
	this->isInitialised = true;
	this->Create();
	if (this->jvm == nullptr)
    {
		ReaDebug::Log("DrivenByMoss: JVM could not be created.\n");
		return;
    }
	JNIEnv* env = this->GetEnv();
	if (env == nullptr)
	{
		ReaDebug::Log("DrivenByMoss: JVM Environment could not be created.\n");
		return;
	}

	ReaDebug::Log("DrivenByMoss: Registering CPP callbacks.\n");
	this->RegisterMethods(*env, functions);
    
	if (ENABLE_JAVA_START)
    {
        ReaDebug::Log("DrivenByMoss: Starting application.\n");
		this->StartApp(*env);
    }
    
    ReaDebug::Log("DrivenByMoss: JVM startup finished.\n");
}


/**
 * Create an instance of the Java Virtual Machine.
 */
void JvmManager::Create()
{
	std::string libDir = GetLibraryPath();
	if (libDir.empty())
		return;
	this->javaHomePath = libDir + "java-runtime";

	if (!LoadJvmLibrary())
		return;

	this->classpath = this->CreateClasspath(libDir);
	if (this->classpath.empty())
		return;

	JavaVMOption* const  opts = this->options.get();
	if (opts == nullptr)
		return;
	DISABLE_WARNING_NO_POINTER_ARITHMETIC
	DISABLE_WARNING_USE_GSL_AT
	opts[0].optionString = &this->classpath[0];
	if (this->debug)
	{
		DISABLE_WARNING_NO_POINTER_ARITHMETIC
		DISABLE_WARNING_USE_GSL_AT
		opts[1].optionString = &this->jvmCmdOptions[0];
	}

	// Minimum required Java version
	JavaVMInitArgs vm_args{};
	vm_args.version = CURRENT_JNI_VERSION;
	vm_args.nOptions = this->debug ? 2 : 1;
	vm_args.options = this->options.get();
	// Invalid options make the JVM init fail
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Load and initialize Java VM and JNI interface
	// NOTE: SEGV (or exception 0xC0000005) is generated intentionally on JVM startup 
	// to verify certain CPU/OS features! Advice debugger to skip it.
	// In Xcode: Create a breakpoint, edit it, add action, enter the line: pro hand -p true -s false SIGSEGV
	// Check option: Automatically continue...
	DISABLE_WARNING_NO_C_STYLE_CONVERSION
	jint(*JNI_CreateJavaVM)(JavaVM**, void**, void*) = (jint(*)(JavaVM**, void**, void*))
#ifdef _WIN32
		GetProcAddress
#else
		dlsym
#endif
		(this->jvmLibHandle, "JNI_CreateJavaVM");
	if (JNI_CreateJavaVM == nullptr)
	{
		ReaDebug() << "ERROR: Could not lookup CreateJavaVm function in library with " << classpath;
		return;
	}
	DISABLE_WARNING_MARK_AS_NOT_NULL
	JNIEnv* env = nullptr;
	DISABLE_WARNING_REINTERPRET_CAST
	// Note: If the next line crashes in debugger make sure that jdwp.dll and dt_socket.dll are from the same JDK!
	// Simply copy the whole JDK
	const jint rc = JNI_CreateJavaVM(&this->jvm, reinterpret_cast<void**> (&env), &vm_args);
	if (rc != JNI_OK)
	{
		ReaDebug() << "ERROR: Could not start Java Virtual Machine. Error code: " << rc << " Classpath: " << classpath;
		return;
	}

	RetrieveMethods(*env);
}


/**
 * Looks up the Java Virtual Machine library inside of the library folder (it is now bundled with DrivenByMoss).
 *
 * @return True on success
 */
bool JvmManager::LoadJvmLibrary()
{
	if (this->javaHomePath.empty())
		return false;

	std::string libPath = LookupJvmLibrary(this->javaHomePath);
	if (libPath.empty())
	{
		ReaDebug() << "Could not find Java dynamic library.";
		return false;
	}

	std::string error{ "" };
#ifdef _WIN32
	this->jvmLibHandle = LoadLibrary(stringToWs(libPath).c_str());
	std::ostringstream ss;
	ss << libPath.c_str() << " Error Code: " << GetLastError();
	error = ss.str();
#else
	this->jvmLibHandle = dlopen(libPath.c_str(), RTLD_NOW);
    char *message = dlerror();
	if (message != nullptr)
        error = message;
#endif
	if (!this->jvmLibHandle)
	{
		ReaDebug() << "Could not load Java dynamic library: " << error;
		return false;
	}
	return true;
}


/**
 * Tests several options to find the JVBM library in the JAVA_HOME folder.
 */
std::string JvmManager::LookupJvmLibrary(const std::string& javaHomePathStr) const
{
#ifdef _WIN32
	std::string libPath = javaHomePathStr + "\\bin\\server\\jvm.dll";
#elif LINUX
	std::string libPath = javaHomePathStr + "/lib/server/libjvm.so";
#else
	std::string libPath = javaHomePathStr + "/lib/libjli.dylib";
#endif
	std::ifstream in(libPath);
	if (in.good())
		return libPath;
	ReaDebug() << "Java dynamic library not found at " << libPath;
	return "";
}


/**
 * Register the native Java methods in the JVM.
 *
 * @param env The JNI environment
 * @param functions The C++ callback functions to register with JNI
 */
void JvmManager::RegisterMethods(JNIEnv& env, void* functions[])
{
	const JNINativeMethod methods[]
	{
		{ (char*)"processNoArg", (char*)"(Ljava/lang/String;Ljava/lang/String;)V", functions[0] },
		{ (char*)"processStringArg", (char*)"(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", functions[1] },
		{ (char*)"processStringArgs", (char*)"(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;)V", functions[2] },
		{ (char*)"processIntArg", (char*)"(Ljava/lang/String;Ljava/lang/String;I)V", functions[3] },
		{ (char*)"processDoubleArg", (char*)"(Ljava/lang/String;Ljava/lang/String;D)V", functions[4] },
		{ (char*)"enableUpdates", (char*)"(Ljava/lang/String;Z)V", functions[5] },
		{ (char*)"delayUpdates", (char*)"(Ljava/lang/String;)V", functions[6] },
		{ (char*)"processMidiArg", (char*)"(IIII)V", functions[7] },
		{ (char*)"getMidiInputs", (char*)"()Ljava/util/Map;",functions[8] },
		{ (char*)"getMidiOutputs", (char*)"()Ljava/util/Map;",functions[9] },
		{ (char*)"openMidiInput", (char*)"(I)Z", functions[10] },
		{ (char*)"openMidiOutput", (char*)"(I)Z", functions[11] },
		{ (char*)"closeMidiInput", (char*)"(I)V", functions[12] },
		{ (char*)"closeMidiOutput", (char*)"(I)V", functions[13] },
		{ (char*)"sendMidiData", (char*)"(I[B)V", functions[14] },
		{ (char*)"setNoteInputFilters", (char*)"(II[Ljava/lang/String;)V", functions[15] },
		{ (char*)"setNoteInputKeyTranslationTable", (char*)"(II[I)V", functions[16] },
		{ (char*)"setNoteInputVelocityTranslationTable", (char*)"(II[I)V", functions[17] }
	};

	jclass mainFrameClass = env.FindClass("de/mossgrabers/reaper/MainApp");
	if (mainFrameClass == nullptr)
	{
		ReaDebug() << "MainFrame.class could not be retrieved.";
		return;
	}
	DISABLE_WARNING_ARRAY_POINTER_DECAY
	const int result = env.RegisterNatives(mainFrameClass, methods, sizeof(methods) / sizeof(*methods));
	if (result != 0)
		this->HandleException(env, "ERROR: Could not register native functions");
}


/**
 * Call the startup method in the main class of the JVM.
 * 
 * @param env The JNI environment
 */
void JvmManager::StartApp(JNIEnv& env)
{
	jstring iniPath = env.NewStringUTF(GetResourcePath());
	jstring appVersion = env.NewStringUTF(GetAppVersion());
	env.CallStaticVoidMethod(this->controllerClass, this->methodIDStartup, iniPath, appVersion);
	this->HandleException(env, "ERROR: Could not call startup.");
}


/**
 * Call the start infrastructure method in the main class of the JVM.
 */
void JvmManager::StartInfrastructure()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDAddDevice == nullptr || this->methodIDStartInfrastructure ==nullptr)
		return;

	// Send all available FX
	const char* name{ nullptr };
	const char* ident{ nullptr };
	int count = 0;
	while (EnumInstalledFX(count, &name, &ident))
	{
		jstring jName = env->NewStringUTF(name);
		jstring jIdent = env->NewStringUTF(ident);
		env->CallStaticVoidMethod(controllerClass, this->methodIDAddDevice, jName, jIdent);
		this->HandleException(*env, "ERROR: Could not call addDevice.");
		// Prevent local reference accumulation
		env->DeleteLocalRef(jName);
		env->DeleteLocalRef(jIdent);
		count++;
	}

	// Call main start method
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDStartInfrastructure);
	this->HandleException(*env, "ERROR: Could not call startInfrastructure.");
}


/**
 * Call the displayWindow method in the main class of the JVM.
 */
void JvmManager::DisplayWindow()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDDisplayWindow == nullptr)
		return;
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDDisplayWindow);
	this->HandleException(*env, "ERROR: Could not call displayWindow.");
}


/**
 * Call the displayProjectWindow method in the main class of the JVM.
 */
void JvmManager::DisplayProjectWindow()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDDisplayProjectWindow == nullptr)
		return;
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDDisplayProjectWindow);
	this->HandleException(*env, "ERROR: Could not call displayProjectWindow.");
}


/**
 * Call the displayParameterWindow method in the main class of the JVM.
 */
void JvmManager::DisplayParameterWindow()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDDisplayParameterWindow == nullptr)
		return;
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDDisplayParameterWindow);
	this->HandleException(*env, "ERROR: Could not call displayParameterWindow.");
}


/**
 * Call the restartControllers method in the main class of the JVM.
 */
void JvmManager::RestartControllers()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDResetController == nullptr)
		return;
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDResetController);
	this->HandleException(*env, "ERROR: Could not call restartControllers.");
}


/**
 * Call the updateModel method in the main class of the JVM.
 *
 * @param data The data to send
 */
void JvmManager::UpdateModel(const std::string& data)
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || methodIDUpdateModel == nullptr)
		return;
	jstring dataUTF = env->NewStringUTF(data.c_str());
	env->CallStaticVoidMethod(this->controllerClass, methodIDUpdateModel, dataUTF);
	this->HandleException(*env, "ERROR: Could not call updateModel.");
}


/**
 * Call the setDefaultDocumentSettings method in the main class of the JVM.
 */
void JvmManager::SetDefaultDocumentSettings()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDSetDefaultDocumentSettings == nullptr)
		return;
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDSetDefaultDocumentSettings);
	this->HandleException(*env, "ERROR: Could not call setDefaultDocumentSettings.");
}


/**
 * Call the getFormattedDocumentSettings method in the main class of the JVM.
 *
 * @return The document settings formatted to be stored in the Reaper extension data
 */
std::string JvmManager::GetFormattedDocumentSettings()
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDGetFormattedDocumentSettings == nullptr)
		return "";
	jstring jdata = (jstring)env->CallStaticObjectMethod(this->controllerClass, this->methodIDGetFormattedDocumentSettings);
	if (jdata == nullptr)
		return "";
	this->HandleException(*env, "ERROR: Could not call getFormattedDocumentSettings.");

	jboolean isCopy;
	const char* data = env->GetStringUTFChars(jdata, &isCopy);
	if (data == nullptr)
		return "";
	std::string result{ data };
	env->ReleaseStringUTFChars(jdata, data);
	return result;
}


/**
 * Call the setFormattedDocumentSettings method in the main class of the JVM.
 *
 * @param data The data to send
 */
void JvmManager::SetFormattedDocumentSettings(const std::string& data)
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr || this->methodIDSetFormattedDocumentSettings == nullptr)
		return;
	jstring dataUTF = env->NewStringUTF(data.c_str());
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDSetFormattedDocumentSettings, dataUTF);
	this->HandleException(*env, "ERROR: Could not call setFormattedDocumentSettings.");
}


/**
 * Call the onMIDIEvent method in the main class of the JVM.
 *
 * @param deviceID The ID of the MIDI device/port
 * @param message The data of the MIDI message
 * @param size The number of bytes of the MIDI message
 */
DISABLE_WARNING_CAN_BE_CONST
void JvmManager::OnMIDIEvent(int deviceID, unsigned char* message, int size)
{
	JNIEnv* env = this->GetEnv();
	if (env == nullptr)
		return;
	jbyteArray jMessage = env->NewByteArray(size);
	if (jMessage == nullptr)
		return;
	env->SetByteArrayRegion(jMessage, 0, size, reinterpret_cast<jbyte*>(message));
	env->CallStaticVoidMethod(this->controllerClass, this->methodIDOnMIDIEvent, deviceID, jMessage);
	env->DeleteLocalRef(jMessage);
	this->HandleException(*env, "ERROR: Could not call onMIDIEvent.");
}


/**
 * Get the full path to the DLL. Uses the trick to retrieve it from a local function.
 *
 * Note: Don't make this a class member!
 *
 * @return The full path or empty if error
 */
std::string getDylibPath()
{
#ifdef _WIN32
	// Long paths might be 65K on Window 10 but the path we are after should never be longer than 260
	wchar_t path[MAX_PATH + 1] = {};
	HMODULE hm = NULL;
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&getDylibPath, &hm))
		return std::string{};
	GetModuleFileName(hm, path, MAX_PATH);
	return wstringToDefaultPlatformEncoding(path);
#else
	Dl_info info;
	if (dladdr((void*)getDylibPath, &info) == 0)
		return std::string{};
	return std::string{ info.dli_fname };
#endif
}


/**
 * Create the classpath from all JAR files found in the drivenbymoss library folder.
 *
 * @param libDir The location where the Reaper extension libraries are located
 * @return The full classpath including the VM parameter
 */
std::string JvmManager::CreateClasspath(const std::string& libDir) const
{
	if (libDir.empty())
		return libDir;

#ifdef _WIN32
	const int status = _chdir(libDir.c_str());
#else
	const int status = chdir(libDir.c_str());
#endif
	if (status < 0)
	{
		ReaDebug() << "ERROR: Could not change current directory to " << libDir;
		return "";
	}

	const std::string path = libDir + "drivenbymoss-libs";
	std::ostringstream stream;
	for (const std::string& file : this->GetDirectoryFiles(path))
	{
		if (this->HasEnding(file, ".jar"))
		{
			stream << path << "/" << file;
#ifdef _WIN32
			stream << ";";
#else
			stream << ":";
#endif
		}
	}
	std::string classpathStr = stream.str();
	if (classpathStr.empty())
	{
		ReaDebug() << "No JAR files found in library path: " << path;
		return classpathStr;
	}
	std::string result{ "-Djava.class.path=" + classpathStr.substr(0, classpathStr.length() - 1) };
	return result;
}


/**
 * Get the full path from which the DLL/dynlib was loaded.
 *
 * @return The path or error if empty
 */
std::string JvmManager::GetLibraryPath() const
{
#ifdef DEBUG
	// Used on Mac if running in the debugger since the dylib location is temporary
	// return "/Users/mos/Library/Application Support/REAPER/UserPlugins/";
#endif

	const std::string fullLibPath = getDylibPath();
	if (fullLibPath.empty())
	{
		ReaDebug() << "Could not retrieve library path.";
		return "";
	}

#ifdef _WIN32
	const std::string dylibName{ "reaper_drivenbymoss.dll" };
#elif LINUX
	const std::string dylibName{ "reaper_drivenbymoss.so" };
#else
	const std::string dylibName{ "reaper_drivenbymoss.dylib" };
#endif
	const size_t pathLength = fullLibPath.size();
	const size_t libLength = dylibName.size();
	return fullLibPath.substr(0, pathLength - libLength);
}


/**
 * Get all files in the given directory.
 *
 * @param dir The directory
 * @return All found files (includes "." and "..")
 */
std::vector<std::string> JvmManager::GetDirectoryFiles(const std::string& dir) const
{
	std::vector<std::string> files{};
#ifdef _WIN32
	std::string pattern(dir);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(stringToWs(pattern).c_str(), &data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		ReaDebug() << "Error looking up JAR files in: " << pattern;
		return files;
	}
	do
	{
		std::string file{ wstringToDefaultPlatformEncoding(data.cFileName) };
		files.push_back(file);
	} while (FindNextFile(hFind, &data) != 0);
	FindClose(hFind);
#else
	std::shared_ptr<DIR> directory_ptr(opendir(dir.c_str()), [](DIR* dir) { dir&& closedir(dir); });
	struct dirent* dirent_ptr;
	if (!directory_ptr)
	{
		ReaDebug() << "Error looking up JAR files in: " << dir;
		return files;
	} while ((dirent_ptr = readdir(directory_ptr.get())) != nullptr)
		files.push_back(std::string(dirent_ptr->d_name));
#endif
	return files;
}


// Create a new java.util.TreeMap instance
jobject JvmManager::CreateTreeMap(JNIEnv& env)
{
	jobject object = env.NewObject(this->treeMapClass, this->treeMapConstructor);
	this->HandleException(env, "ERROR: Could not create TreeMap.");
	return object;
}


// Helper function to add entries to the TreeMap
void JvmManager::AddToTreeMap(JNIEnv& env, jobject treeMap, jint key, const std::string& value)
{
	jobject keyObject = env.NewObject(this->integerClass, this->integerConstructor, key);
	jstring valueObject = env.NewStringUTF(value.c_str());
	env.CallObjectMethod(treeMap, this->treeMapPutMethod, keyObject, valueObject);
	this->HandleException(env, "ERROR: Could not add entry to TreeMap.");
}



/**
 * Test if the given string ends with the end-string.
 * @param str The string to test
 * @param end The string to test if str ends with it
 * @return True if str ends with end
 */
bool JvmManager::HasEnding(std::string const& str, std::string const& end) const
{
	return str.length() < end.length() ? false : str.compare(str.length() - end.length(), end.length(), end) == 0;
}


/**
 * Check if a Java exception occured and log it.
 *
 * @param env The JNI environment
 * @param message The error message to display
 */
void JvmManager::HandleException(JNIEnv& env, const char* message) const
{
	if (!env.ExceptionCheck())
		return;
	jthrowable ex = env.ExceptionOccurred();
	ReaDebug dbg{};
	dbg << message;
	if (ex)
	{
		const jmethodID methodID = env.GetMethodID(env.FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
		// jstring is not a real subclass of jobject
		DISABLE_WARNING_NO_STATIC_DOWNCAST
		const jstring s = static_cast<jstring>(env.CallObjectMethod(ex, methodID));
		jboolean isCopy;
		const char* data = env.GetStringUTFChars(s, &isCopy);
		if (data != nullptr)
			dbg << "\n" << data;
		env.ExceptionClear();
	}
}


/**
 * the IDs returned for a given class don't change for the lifetime of the JVM process.
 * But the call to get the field or method can require significant work in the JVM, 
 * because fields and methods might have been inherited from superclasses, making the 
 * JVM walk up the class hierarchy to find them. Because the IDs are the same for a given 
 * class, you should look them up once and then reuse them. Similarly, looking up class 
 * objects can be expensive, so they should be cached as well.
 */
void JvmManager::RetrieveMethods(JNIEnv& env)
{
	this->controllerClass = env.FindClass("de/mossgrabers/reaper/Controller");
	if (this->controllerClass == nullptr)
	{
		ReaDebug() << "Controller.class could not be retrieved.";
		return;
	}

	this->methodIDShutdown = this->RetrieveMethod(env, "shutdown", "()V");
	if (this->methodIDShutdown == nullptr)
		return;

	this->methodIDStartup = this->RetrieveMethod(env, "startup", "(Ljava/lang/String;Ljava/lang/String;)V");
	if (this->methodIDStartup == nullptr)
		return;

	this->methodIDAddDevice = this->RetrieveMethod(env, "addDevice", "(Ljava/lang/String;Ljava/lang/String;)V");
	if (this->methodIDAddDevice == nullptr)
		return;

	this->methodIDStartInfrastructure = this->RetrieveMethod(env, "startInfrastructure", "()V");
	if (this->methodIDStartInfrastructure == nullptr)
		return;

	this->methodIDDisplayWindow = this->RetrieveMethod(env, "displayWindow", "()V");
	if (this->methodIDDisplayWindow == nullptr)
		return;

	this->methodIDDisplayProjectWindow = this->RetrieveMethod(env, "displayProjectWindow", "()V");
	if (this->methodIDDisplayProjectWindow == nullptr)
		return;

	this->methodIDDisplayParameterWindow = this->RetrieveMethod(env, "displayParameterWindow", "()V");
	if (this->methodIDDisplayParameterWindow == nullptr)
		return;

	this->methodIDResetController = this->RetrieveMethod(env, "restartControllers", "()V");
	if (this->methodIDResetController == nullptr)
		return;

	this->methodIDUpdateModel = this->RetrieveMethod(env, "updateModel", "(Ljava/lang/String;)V");
	if (this->methodIDUpdateModel == nullptr)
		return;

	this->methodIDSetDefaultDocumentSettings = this->RetrieveMethod(env, "setDefaultDocumentSettings", "()V");
	if (this->methodIDSetDefaultDocumentSettings == nullptr)
		return;

	this->methodIDGetFormattedDocumentSettings = this->RetrieveMethod(env, "getFormattedDocumentSettings", "()Ljava/lang/String;");
	if (this->methodIDGetFormattedDocumentSettings == nullptr)
		return;

	this->methodIDSetFormattedDocumentSettings = this->RetrieveMethod(env, "setFormattedDocumentSettings", "(Ljava/lang/String;)V");
	if (this->methodIDSetFormattedDocumentSettings == nullptr)
		return;

	this->methodIDOnMIDIEvent = this->RetrieveMethod(env, "onMIDIEvent", "(I[B)V");
	if (this->methodIDOnMIDIEvent == nullptr)
		return;

	// Basic Java classes and their methods
	this->treeMapClass = env.FindClass("java/util/TreeMap");
	if (this->treeMapClass != nullptr)
	{
		this->treeMapConstructor = env.GetMethodID(treeMapClass, "<init>", "()V");
		this->treeMapPutMethod = env.GetMethodID(treeMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	}

	this->integerClass = env.FindClass("java/lang/Integer");
	if (this->integerClass != nullptr)
		this->integerConstructor = env.GetMethodID(integerClass, "<init>", "(I)V");
}


jmethodID JvmManager::RetrieveMethod(JNIEnv& env, const char* name, const char* signature)
{
	jmethodID methodID = env.GetStaticMethodID(this->controllerClass, name, signature);
	if (methodID == nullptr)
	{
		ReaDebug() << name << " method could not be retrieved.";
		return nullptr;
	}
	return methodID;
}


JNIEnv* JvmManager::GetEnv()
{
	if (this->jvm == nullptr)
		return nullptr;

	// This is correct since it is a pointer to a pointer!
	JNIEnv* env = nullptr;
	const jint result = this->jvm->GetEnv(reinterpret_cast<void**>(&env), CURRENT_JNI_VERSION);
	if (result == JNI_OK)
		return env;

	if (result == JNI_EDETACHED)
	{
		// The JNI specification guarantees that JavaVM::AttachCurrentThread() is thread-safe. 
		// Each native thread can safely call it, even concurrently with others.
		if (this->jvm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr) != 0)
		{
			ReaDebug::Log("DrivenByMoss: Could not attach current thread to JVM!\n");
			return nullptr;
		}
		return env;
	}

	// JNI_EVERSION or other fatal error
	return nullptr;
}