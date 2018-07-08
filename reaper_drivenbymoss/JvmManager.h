// Written by Jürgen Moßgraber - mossgrabers.de
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

	void Create(const std::string &currentPath);
	void RegisterMethods(void *processNoArgCPP, void *processStringArgCPP, void *processIntArgCPP, void *processDoubleArgCPP, void *receiveModelDataCPP);
	void StartApp();

private:
	// Pointer to the JVM (Java Virtual Machine)
	JavaVM * jvm;

	// Pointer to native interface
	JNIEnv *env;

	// JVM invocation options
	std::unique_ptr<JavaVMOption[]> options;

	bool debug;
};
