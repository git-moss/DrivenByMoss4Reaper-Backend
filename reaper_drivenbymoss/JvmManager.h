// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_JVMMANAGER_H_
#define _DBM_JVMMANAGER_H_

#include <string>
#include <memory>
#include <vector>

#include "WrapperJNI.h"


/**
 * Manages the start and stop of a Java Virtual Machine.
 */
class JvmManager
{
public:
	JvmManager(bool enableDebug);
	JvmManager(const JvmManager&) = delete;
	JvmManager& operator=(const JvmManager&) = delete;
	JvmManager(JvmManager&&) = delete;
	JvmManager& operator=(JvmManager&&) = delete;
	~JvmManager();

	void init(void* processNoArgCPP, void* processStringArgCPP, void* processStringArgsCPP, void* processIntArgCPP, void* processDoubleArgCPP, void* enableUpdatesCPP, void* delayUpdatesCPP, void* processMidiArgCPP);

	bool isRunning() const noexcept
	{
		return this->isInitialised && this->jvm != nullptr;
	}

	const std::string& GetJavaHomePath() const noexcept
	{
		return this->javaHomePath;
	}

	void DisplayWindow();
	void DisplayProjectWindow();
	void UpdateModel(const std::string& data);

	void SetDefaultDocumentSettings();
	std::string GetFormattedDocumentSettings();
	void SetFormattedDocumentSettings(const std::string& data);

private:
	std::string javaHomePath;
	std::string jvmCmdOptions;
	std::string classpath;

	// Pointer to the JVM (Java Virtual Machine)
	JavaVM* jvm;

	// Pointer to native interface
	JNIEnv* env;

	// JVM invocation options
	std::unique_ptr<JavaVMOption[]> options;

#ifdef _WIN32
	HMODULE jvmLibHandle;
#else
	void* jvmLibHandle;
#endif

	bool debug;
	bool isInitialised{ false };
	jclass controllerClass{ nullptr };


	void Create();
	void RegisterMethods(void* processNoArgCPP, void* processStringArgCPP, void* processStringArgsCPP, void* processIntArgCPP, void* processDoubleArgCPP, void* enableUpdatesCPP, void* delayUpdatesCPP, void* processMidiArgCPP);
	void StartApp();

	bool LoadJvmLibrary();
	std::string LookupJvmLibrary(const std::string& javaHomePath) const;
	std::string CreateClasspath(const std::string& libDir) const;
	std::vector<std::string> GetDirectoryFiles(const std::string& dir) const;
	std::string GetLibraryPath() const;

	jclass GetControllerClass() noexcept;
	void HandleException(const char* message) const;
	bool HasEnding(std::string const& fullString, std::string const& ending) const;
};

#endif /* _DBM_JVMMANAGER_H_ */