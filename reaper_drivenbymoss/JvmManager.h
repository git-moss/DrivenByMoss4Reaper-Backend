// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <cstring>
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

    void init (void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP);
    
    bool isRunning()
    {
        return this->isInitialised && this->jvm != nullptr;
    }

private:
	// Pointer to the JVM (Java Virtual Machine)
	JavaVM * jvm;

	// Pointer to native interface
	JNIEnv *env;

	// JVM invocation options
	std::unique_ptr<JavaVMOption[]> options;

	bool debug;
	bool isInitialised = false;


	void Create();
	void RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP);
	void StartApp();
	std::string CreateClasspath() const;
	std::vector<std::string> GetDirectoryFiles(const std::string &dir) const;
    std::string GetLibraryPath() const;
	bool HasEnding(std::string const &fullString, std::string const &ending) const;
};
