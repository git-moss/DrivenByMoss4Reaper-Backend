// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <cstring>
#include <jni.h>


/**
 * Manages the start and stop of a Java Virtual Machine.
 */
class JvmManager
{
public:
	JvmManager(bool enableDebug);
	~JvmManager();

	void init (const std::string &currentPath, void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP)
	{
		if (this->isInitialised)
			return;
		this->isInitialised = true;
		this->Create(currentPath);
		this->RegisterMethods(processNoArgCPP, processStringArgCPP, processIntArgCPP, processDoubleArgCPP, receiveModelDataCPP);
		this->StartApp();
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


	void Create(const std::string &currentPath);
	void RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP);
	void StartApp();
};
