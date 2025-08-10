// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_JVMMANAGER_H_
#define _DBM_JVMMANAGER_H_

#include <string>
#include <memory>
#include <vector>

#include "CodeAnalysis.h"
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

	void Init(void* functions[]);

	bool IsRunning() const noexcept
	{
		return this->isInitialised && this->jvm != nullptr;
	}

	const std::string& GetJavaHomePath() const noexcept
	{
		return this->javaHomePath;
	}

	void DisplayWindow();
	void DisplayProjectWindow();
	void DisplayParameterWindow();
	void RestartControllers();
	void UpdateModel(const std::string& data);

	void StartInfrastructure();
	void SetCleanShutdown() noexcept { this->isCleanShutdown = true; };

	void SetDefaultDocumentSettings();
	std::string GetFormattedDocumentSettings();
	void SetFormattedDocumentSettings(const std::string& data);

	jobject CreateTreeMap(JNIEnv& env);
	void AddToTreeMap(JNIEnv& env, jobject treeMap, jint key, const std::string& value);

	void OnMIDIEvent(int deviceID, unsigned char* message, int size);

private:
	std::string javaHomePath;
	std::string jvmCmdOptions;
	std::string classpath;

	// Pointer to the JVM (Java Virtual Machine)
	JavaVM* jvm;

	// JVM invocation options
	std::unique_ptr<JavaVMOption[]> options;

#ifdef _WIN32
	HMODULE jvmLibHandle;
#else
	void* jvmLibHandle;
#endif

	bool debug;
	bool isInitialised{ false };
	bool isCleanShutdown{ false };

	jclass treeMapClass{ nullptr };
	jmethodID treeMapConstructor{ nullptr };
	jmethodID treeMapPutMethod{ nullptr };
	jclass integerClass{ nullptr };
	jmethodID integerConstructor{ nullptr };

	jclass controllerClass{ nullptr };
	jmethodID methodIDShutdown{ nullptr };
	jmethodID methodIDStartup{ nullptr };
	jmethodID methodIDAddDevice{ nullptr };
	jmethodID methodIDStartInfrastructure{ nullptr };
	jmethodID methodIDDisplayWindow{ nullptr };
	jmethodID methodIDDisplayProjectWindow{ nullptr };
	jmethodID methodIDDisplayParameterWindow{ nullptr };
	jmethodID methodIDResetController{ nullptr };
	jmethodID methodIDUpdateModel{ nullptr };
	jmethodID methodIDSetDefaultDocumentSettings{ nullptr };
	jmethodID methodIDGetFormattedDocumentSettings{ nullptr };
	jmethodID methodIDSetFormattedDocumentSettings{ nullptr };
	jmethodID methodIDOnMIDIEvent{ nullptr };

	void Create();
	DISABLE_WARNING_ARRAY_POINTER_DECAY
	void RegisterMethods(JNIEnv& env, void* functions[]);
	void StartApp(JNIEnv& env);

	bool LoadJvmLibrary();
	std::string LookupJvmLibrary(const std::string& theJavaHomePath) const;
	std::string CreateClasspath(const std::string& libDir) const;
	std::vector<std::string> GetDirectoryFiles(const std::string& dir) const;
	std::string GetLibraryPath() const;

	void HandleException(JNIEnv& env, const char* message) const;
	bool HasEnding(std::string const& fullString, std::string const& ending) const;

	JNIEnv* JvmManager::GetEnv();
	void RetrieveMethods(JNIEnv& env);
	jmethodID RetrieveMethod(JNIEnv& env, const char* name, const char* signature);
};

#endif /* _DBM_JVMMANAGER_H_ */
