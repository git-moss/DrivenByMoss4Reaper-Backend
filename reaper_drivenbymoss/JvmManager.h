// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <memory>
#include <jni.h>


/**
 * Manages the start and stop of a Java Virtual Machine.
 */
class JvmManager
{
public:
	JvmManager(bool enableDebug);
	~JvmManager();

    void init (void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *processMidiArgCPP);
    
    bool isRunning() const
    {
        return this->isInitialised && this->jvm != nullptr;
    }

	const std::string &GetJavaHomePath() const
	{
		return this->javaHomePath;
	}

	void DisplayWindow();
	void UpdateModel(std::string data);

private:
	std::string javaHomePath;

	// Pointer to the JVM (Java Virtual Machine)
	JavaVM *jvm;

	// Pointer to native interface
	JNIEnv *env;

	// JVM invocation options
	std::unique_ptr<JavaVMOption[]> options;

#ifdef _WIN32
	HMODULE jvmLibHandle;
#else
	void *jvmLibHandle;
#endif

	bool debug;
	bool isInitialised{ false };
	jclass controllerClass{ nullptr };


	void Create();
	void RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *processMidiArgCPP);
	void StartApp();

	bool LoadJvmLibrary();
	std::string LookupJvmLibrary(const std::string &javaHomePath) const;
	std::string CreateClasspath(std::string libDir) const;
	std::vector<std::string> GetDirectoryFiles(const std::string &dir) const;
    std::string GetLibraryPath() const;

	jclass GetControllerClass();
	void HandleException(const char *message) const;
	bool HasEnding(std::string const &fullString, std::string const &ending) const;
};
