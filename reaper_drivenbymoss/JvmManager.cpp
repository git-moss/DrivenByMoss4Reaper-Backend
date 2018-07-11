// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <dlfcn.h>
#endif

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

#include "JvmManager.h"
#include "ReaDebug.h"
#include "reaper_plugin_functions.h"


/**
 * Constructor.
 *
 * @param enableDebug True to enable debugging
 */
JvmManager::JvmManager(bool enableDebug) : jvm(nullptr), env(nullptr)
{
	this->debug = enableDebug;
	this->options = std::make_unique<JavaVMOption[]>(this->debug ? 4 : 1);
}


/**
 * Destructor.
 */
JvmManager::~JvmManager()
{
	if (this->jvm != nullptr)
	{
		jclass transformatorClass = env->FindClass("de/mossgrabers/transformator/Transformator");
		if (transformatorClass != nullptr)
		{
			jmethodID mid = env->GetStaticMethodID(transformatorClass, "shutdown", "()V");
			if (mid != nullptr)
				env->CallStaticVoidMethod(transformatorClass, mid);
		}
		this->jvm->DestroyJavaVM();
		this->jvm = nullptr;
	}
	this->options.reset();
	this->env = nullptr;
}


/**
 * Start and initialise the JVM.
 *
 * @param processNoArgCPP The processing method with no arguments
 * @param processStringArgCPP The processing method with a string argument
 * @param processIntArgCPP The processing method with an integer argument
 * @param processDoubleArgCPP The processing method with a double argument
 * @param receiveModelDataCPP The callback for getting a model update
 */
void JvmManager::init (void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP)
{
    if (this->isInitialised)
        return;
    this->isInitialised = true;
    this->Create();
    if (this->jvm == nullptr)
        return;
    this->RegisterMethods(processNoArgCPP, processStringArgCPP, processIntArgCPP, processDoubleArgCPP, receiveModelDataCPP);
    this->StartApp();
}


/**
 * Create an instance of the Java Virtual Machine.
 */
void JvmManager::Create()
{
	std::string classpath = this->CreateClasspath();
    if (classpath.empty())
        return;

	JavaVMInitArgs vm_args{};
	JavaVMOption * const  opts = this->options.get();
	opts[0].optionString = (char *)classpath.c_str();
	if (this->debug)
	{
		opts[1].optionString = (char *) "-Xdebug";
		opts[2].optionString = (char *) "-Xrunjdwp:transport=dt_socket,address=8989,server=y,suspend=y";
		opts[3].optionString = (char *) "-Xcheck:jni";
	}

	// Minimum required Java version
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = this->debug ? 4 : 1;
	vm_args.options = this->options.get();
	// Invalid options make the JVM init fail
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Load and initialize Java VM and JNI interface
	// NOTE: SEGV (or exception 0xC0000005) is generated intentionally on JVM startup 
	// to verify certain CPU/OS features! Advice debugger to skip it.
	const jint rc = JNI_CreateJavaVM(&this->jvm, reinterpret_cast<void**> (&this->env), &vm_args);
	if (rc != JNI_OK)
	{
		ReaDebug() << "ERROR: Could not start Java Virtual Machine with " << classpath;
		return;
	}
}


/**
 * Register the native Java methods in the JVM.
 *
 * @param processNoArgCPP The processing method with no arguments
 * @param processStringArgCPP The processing method with a string argument
 * @param processIntArgCPP The processing method with an integer argument
 * @param processDoubleArgCPP The processing method with a double argument
 * @param receiveModelDataCPP The callback for getting a model update
 */
void JvmManager::RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP)
{
	const JNINativeMethod methods[]
	{
		{ (char*) "processNoArg", (char*) "(Ljava/lang/String;)V", processNoArgCPP },
		{ (char*) "processStringArg", (char*) "(Ljava/lang/String;Ljava/lang/String;)V", processStringArgCPP },
		{ (char*) "processIntArg", (char*) "(Ljava/lang/String;I)V", processIntArgCPP },
		{ (char*) "processDoubleArg", (char*) "(Ljava/lang/String;D)V", processDoubleArgCPP },
		{ (char*) "receiveModelData", (char*) "(Z)Ljava/lang/String;", receiveModelDataCPP }
	};

	jclass transformatorAppClass = this->env->FindClass("de/mossgrabers/transformator/TransformatorApplication");
	const int result = this->env->RegisterNatives(transformatorAppClass, methods, 5);
	if (result == 0)
		return;
	jthrowable ex = this->env->ExceptionOccurred();
	ReaDebug dbg{};
	dbg << "ERROR: Could not register native functions";
	if (ex)
	{
		jboolean isCopy = false;
		jmethodID toString = this->env->GetMethodID(this->env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
		jstring s = static_cast<jstring>(this->env->CallObjectMethod(ex, toString));
		const char* utf = this->env->GetStringUTFChars(s, &isCopy);
		dbg << utf;
		this->env->ExceptionClear();
	}
}


/**
 * Call the main method in the main class of the JVM.
 */
void JvmManager::StartApp()
{
	jclass transformatorClass = this->env->FindClass("de/mossgrabers/transformator/Transformator");
	if (transformatorClass == nullptr)
		return;
	// Call main start method
	jmethodID methodID = this->env->GetStaticMethodID(transformatorClass, "main", "([Ljava/lang/String;)V");
	if (methodID == nullptr)
		return;
	const char*iniPath = GetResourcePath();
	jobjectArray applicationArgs = this->env->NewObjectArray(1, this->env->FindClass("java/lang/String"), nullptr);
	this->env->SetObjectArrayElement(applicationArgs, 0, this->env->NewStringUTF(iniPath));
	this->env->CallStaticVoidMethod(transformatorClass, methodID, applicationArgs);
}


#ifdef _WIN32
/**
 * Convert a string to wide char.
 */
std::wstring stringToWs(const std::string& s)
{
    int slength = (int)s.length() + 1;
    int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
#endif

/**
 * Get the full path to the DLL. Uses the trick to retrieve it from a local function.
 *
 * @return The full path or empty if error
 */
std::string getDylibPath()
{
#ifdef _WIN32
	// Long paths might be 65K on Window 10 but the path we are after should never be longer than 260
	char path[MAX_PATH];
	HMODULE hm = NULL;
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &getDylibPath, &hm))
		return std::string{};
	GetModuleFileNameA(hm, path, sizeof(path));
	return std::string{ path };
#else
    Dl_info info;
    if (dladdr((void *)getDylibPath, &info) == 0)
        return std::string{};
    return std::string{ info.dli_fname };
#endif
}


/**
 * Create the classpath from all JAR files found in the drivenbymoss library folder.
 *
 * @return The full classpath including the VM parameter
 */
std::string JvmManager::CreateClasspath() const
{
    std::string libDir = GetLibraryPath();
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

    const std::string subdir = "drivenbymoss-libs";
    const std::string path = libDir + subdir;

    std::stringstream stream;
	for (const std::string &file : this->GetDirectoryFiles(path))
	{
		if (this->HasEnding(file, ".jar"))
			stream << "./" << subdir << "/" << file << ";";
	}
	std::string result = stream.str();
    if (result.empty())
    {
        ReaDebug() << "No JAR files found in library path: " << path;
        return result;
    }
	return "-Djava.class.path=" + result.substr(0, result.length() - 1);
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
	// TODO Test if it works with "~/Library/Application Support/REAPER/UserPlugins/"
	return "/Users/mos/Library/Application Support/REAPER/UserPlugins/";
#endif

	const std::string fullLibPath = getDylibPath();
	if (fullLibPath.empty())
	{
		ReaDebug() << "Could not retrieve library path.";
		return "";
	}

#ifdef _WIN32
	const std::string dylibName{ "reaper_drivenbymoss.dll" };
#else
	const std::string dylibName{ "reaper_drivenbymoss.dylib" };
#endif
	return fullLibPath.substr(0, fullLibPath.size() - dylibName.size());
}


/**
 * Get all files in the given directory.
 *
 * @param dir The directory
 * @return All found files (includes "." and "..")
 */
std::vector<std::string> JvmManager::GetDirectoryFiles(const std::string &dir) const
{
	std::vector<std::string> files{};
#ifdef _WIN32
	std::string pattern(dir);
	pattern.append("\\*");
	WIN32_FIND_DATA data;
	HANDLE hFind = hFind = FindFirstFile(stringToWs(pattern).c_str(), &data);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		ReaDebug() << "Error looking up JAR files in: " << pattern;
		return files;
	}
	do
	{
		std::wstring ws(data.cFileName);
		std::string file(ws.begin(), ws.end());
		files.push_back(file);
	} while (FindNextFile(hFind, &data) != 0);
	FindClose(hFind);
#else
	std::shared_ptr<DIR> directory_ptr(opendir(dir.c_str()), [](DIR* dir) { dir && closedir(dir); });
	struct dirent *dirent_ptr;
	if (!directory_ptr)
	{
		ReaDebug() << "Error looking up JAR files in: " << dir;
		return files;
	}
	while ((dirent_ptr = readdir(directory_ptr.get())) != nullptr)
		files.push_back(std::string(dirent_ptr->d_name));
#endif
	return files;
}


/**
 * Test if the given string ends with the end-string.
 * @param str The string to test
 * @param end The string to test if str ends with it
 * @return True if str ends with end
 */
bool JvmManager::HasEnding(std::string const &str, std::string const &end) const
{
	return str.length() < end.length() ? false : str.compare(str.length() - end.length(), end.length(), end) == 0;
}
