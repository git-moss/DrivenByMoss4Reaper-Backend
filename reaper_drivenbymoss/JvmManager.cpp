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

#include <vector>
#include <sstream>
#include <fstream>

#include "JvmManager.h"
#include "ReaDebug.h"
#include "ReaperUtils.h"

#ifdef WIN32
extern std::wstring stringToWs(const std::string& s);
#endif

/**
 * Constructor.
 *
 * @param enableDebug True to enable debugging
 */
JvmManager::JvmManager(bool enableDebug) : jvm(nullptr), env(nullptr), jvmLibHandle(nullptr)
{
	this->debug = enableDebug;
	this->options = std::make_unique<JavaVMOption[]>(this->debug ? 3 : 1);
}


/**
 * Destructor.
 */
JvmManager::~JvmManager()
{
	if (this->jvm != nullptr)
	{
		jclass clazz = this->GetControllerClass();
		if (clazz != nullptr)
		{
			jmethodID mid = env->GetStaticMethodID(clazz, "shutdown", "()V");
			if (mid != nullptr)
			{
				env->CallStaticVoidMethod(clazz, mid);
				this->HandleException("Could not call shutdown.");
			}
		}
		this->jvm = nullptr;
	}
	this->options.reset();
	this->env = nullptr;

	// Unload JVM library
	if (this->jvmLibHandle)
#ifdef _WIN32
		FreeLibrary(this->jvmLibHandle);
#else
		dlclose(this->jvmLibHandle);
#endif
	this->jvmLibHandle = nullptr;
}


/**
 * Start and initialise the JVM.
 *
 * @param processNoArgCPP The processing method with no arguments
 * @param processStringArgCPP The processing method with a string argument
 * @param processIntArgCPP The processing method with an integer argument
 * @param processDoubleArgCPP The processing method with a double argument
 */
void JvmManager::init(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP)
{
	if (this->isInitialised)
		return;
	this->isInitialised = true;
	this->Create();
	if (this->jvm == nullptr)
		return;
	this->RegisterMethods(processNoArgCPP, processStringArgCPP, processIntArgCPP, processDoubleArgCPP);
	this->StartApp();
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

	std::string classpath = this->CreateClasspath(libDir);
	if (classpath.empty())
		return;

	JavaVMOption * const  opts = this->options.get();
	opts[0].optionString = (char *)classpath.c_str();
	if (this->debug)
	{
		opts[1].optionString = (char *) "-Xdebug";
		opts[2].optionString = (char *) "-Xrunjdwp:transport=dt_socket,address=8989,server=y,suspend=y";
	}

	// Minimum required Java version
	JavaVMInitArgs vm_args{};
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = this->debug ? 3 : 1;
	vm_args.options = this->options.get();
	// Invalid options make the JVM init fail
	vm_args.ignoreUnrecognized = JNI_FALSE;

	// Load and initialize Java VM and JNI interface
	// NOTE: SEGV (or exception 0xC0000005) is generated intentionally on JVM startup 
	// to verify certain CPU/OS features! Advice debugger to skip it.
    // In Xcode: Create a breakpoint, edit it, add action, enter the line: pro hand -p true -s false SIGSEGV
    // Check option: Automatically continue...
	jint(*JNI_CreateJavaVM)(JavaVM **, void **, void *) = (jint(*)(JavaVM **, void **, void *))
#ifdef _WIN32
		GetProcAddress(this->jvmLibHandle, "JNI_CreateJavaVM");
#else
		dlsym(this->jvmLibHandle, "JNI_CreateJavaVM");
#endif
	if (JNI_CreateJavaVM == nullptr)
	{
		ReaDebug() << "ERROR: Could not lookup CreateJavaVm function in library with " << classpath;
		return;
	}
	const jint rc = JNI_CreateJavaVM(&this->jvm, reinterpret_cast<void**> (&this->env), &vm_args);
	if (rc != JNI_OK)
	{
		ReaDebug() << "ERROR: Could not start Java Virtual Machine with " << classpath;
		return;
	}
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

#ifdef _WIN32
	this->jvmLibHandle = LoadLibrary(stringToWs(libPath).c_str());
#else
	this->jvmLibHandle = dlopen(libPath.c_str(), RTLD_NOW);
#endif
	if (!this->jvmLibHandle)
	{
		ReaDebug() << "Could not load Java dynamic library.";
		return false;
	}
	return true;
}


/**
 * Tests several options to find the JVBM library in the JAVA_HOME folder.
 */
std::string JvmManager::LookupJvmLibrary(const std::string &javaHomePath) const
{
#ifdef _WIN32
	std::vector<std::string> libSubPaths{ "\\bin\\server\\jvm.dll" };
#elif LINUX
	std::vector<std::string> libSubPaths{ "/lib/server/libjvm.so" };
#else
    std::vector<std::string> libSubPaths{ "/lib/jli/libjli.dylib" };
#endif
	std::string libPath{};
	for (const std::string &p : libSubPaths)
	{
		libPath = javaHomePath + p;
		std::ifstream in(libPath);
		if (in.good())
			return libPath;
	}
	ReaDebug() << "Java dynamic library not found at " << libPath;
	return "";
}


/**
 * Register the native Java methods in the JVM.
 *
 * @param processNoArgCPP The processing method with no arguments
 * @param processStringArgCPP The processing method with a string argument
 * @param processIntArgCPP The processing method with an integer argument
 * @param processDoubleArgCPP The processing method with a double argument
 */
void JvmManager::RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP)
{
	const JNINativeMethod methods[]
	{
		{ (char*) "processNoArg", (char*) "(Ljava/lang/String;)V", processNoArgCPP },
		{ (char*) "processStringArg", (char*) "(Ljava/lang/String;Ljava/lang/String;)V", processStringArgCPP },
		{ (char*) "processIntArg", (char*) "(Ljava/lang/String;I)V", processIntArgCPP },
		{ (char*) "processDoubleArg", (char*) "(Ljava/lang/String;D)V", processDoubleArgCPP }
	};

	jclass mainFrameClass = this->env->FindClass("de/mossgrabers/reaper/ui/MainFrame");
	if (mainFrameClass == nullptr)
    {
        ReaDebug() << "MainFrame.class could not be retrieved.";
        return;
    }
    const int result = this->env->RegisterNatives(mainFrameClass, methods, 4);
	if (result != 0)
		this->HandleException("ERROR: Could not register native functions");
}


/**
 * Call the startup method in the main class of the JVM.
 */
void JvmManager::StartApp()
{
	jclass clazz = this->GetControllerClass();
	if (clazz == nullptr)
		return;
	// Call main start method
	jmethodID methodID = this->env->GetStaticMethodID(clazz, "startup", "(Ljava/lang/String;)V");
	if (methodID == nullptr)
		return;
	jstring iniPath = this->env->NewStringUTF(GetResourcePath());
	this->env->CallStaticVoidMethod(clazz, methodID, iniPath);
	this->HandleException("ERROR: Could not call startup.");
}


/**
 * Call the displayWindow method in the main class of the JVM.
 */
void JvmManager::DisplayWindow()
{
	jclass clazz = this->GetControllerClass();
	if (clazz == nullptr)
		return;
	// Call displayWindow method
	jmethodID methodID = this->env->GetStaticMethodID(clazz, "displayWindow", "()V");
	if (methodID == nullptr)
		return;
	this->env->CallStaticVoidMethod(clazz, methodID);
	this->HandleException("ERROR: Could not call displayWindow.");
}


/**
 * Call the updateModel method in the main class of the JVM.
 *
 * @param data The data to send
 */
void JvmManager::UpdateModel(std::string data)
{
	jclass clazz = this->GetControllerClass();
	if (clazz == nullptr)
		return;
	// Call updateModel method
	jmethodID methodID = this->env->GetStaticMethodID(clazz, "updateModel", "(Ljava/lang/String;)V");
	if (methodID == nullptr)
		return;
	jstring dataUTF = this->env->NewStringUTF(data.c_str());
	this->env->CallStaticVoidMethod(clazz, methodID, dataUTF);
	this->HandleException("ERROR: Could not call updateModel.");
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
	char path[MAX_PATH];
	HMODULE hm = NULL;
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&getDylibPath, &hm))
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
 * @param libDir The location where the Reaper extension libraries are located
 * @return The full classpath including the VM parameter
 */
std::string JvmManager::CreateClasspath(std::string libDir) const
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

	const std::string subdir = "drivenbymoss-libs";
	const std::string path = libDir + subdir;

	std::stringstream stream;
	for (const std::string &file : this->GetDirectoryFiles(path))
	{
		if (this->HasEnding(file, ".jar"))
        {
            stream << path << "/" << file;
#ifdef _WIN32
            stream<< ";";
#else
            stream<< ":";
#endif
        }
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


/**
 * Check if a Java exception occured and log it.
 *
 * @param message The error message to display
 */
void JvmManager::HandleException(const char *message) const
{
	if (!this->env->ExceptionCheck())
		return;
	jthrowable ex = this->env->ExceptionOccurred();
	ReaDebug dbg{};
	dbg << message;
	if (ex)
	{
		const jmethodID toString = this->env->GetMethodID(this->env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
		const jstring s = static_cast<jstring>(this->env->CallObjectMethod(ex, toString));
		jboolean isCopy = false;
		dbg << this->env->GetStringUTFChars(s, &isCopy);
		this->env->ExceptionClear();
	}
}


jclass JvmManager::GetControllerClass()
{
	if (this->controllerClass != nullptr)
		return this->controllerClass;
	this->controllerClass = this->env->FindClass("de/mossgrabers/reaper/Controller");
	if (this->controllerClass == nullptr)
	{
		ReaDebug() << "Controller.class could not be retrieved.";
		return nullptr;
	}
	return this->controllerClass;
}
