// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifdef _WIN32
#include <direct.h>
#elif
#include <unistd.h>
#endif

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include "JvmManager.h"
#include "reaper_plugin_functions.h"


/**
 * Constructor.
 *
 * @param enableDebug True to enable debugging
 */
JvmManager::JvmManager(bool enableDebug) : jvm(nullptr), env(nullptr)
{
	this->debug = enableDebug;
	this->options = std::make_unique<JavaVMOption []> (this->debug ? 4 : 1);
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
	}
	this->options.reset();
}


/**
 * Create an instance of the Java Virtual Machine.
 *
 * @param currentPath The path where the drivenbymoss-libs folder is located
 */
void JvmManager::Create(const std::string &currentPath)
{
	const int result = _chdir(currentPath.c_str());
	if (result < 0)
	{
		ReaScriptError("ERROR: Could not change current directory!");
		return;
	}

	JavaVMInitArgs vm_args{};
	JavaVMOption * const  opts = this->options.get();
	// TODO create from given folder...
	opts[0].optionString = (char *) "-Djava.class.path=./drivenbymoss-libs/batik-anim-1.9.1.jar;./drivenbymoss-libs/batik-awt-util-1.9.1.jar;./drivenbymoss-libs/batik-bridge-1.9.1.jar;./drivenbymoss-libs/batik-constants-1.9.1.jar;./drivenbymoss-libs/batik-css-1.9.1.jar;./drivenbymoss-libs/batik-dom-1.9.1.jar;./drivenbymoss-libs/batik-ext-1.9.1.jar;./drivenbymoss-libs/batik-gvt-1.9.1.jar;./drivenbymoss-libs/batik-i18n-1.9.1.jar;./drivenbymoss-libs/batik-parser-1.9.1.jar;./drivenbymoss-libs/batik-script-1.9.1.jar;./drivenbymoss-libs/batik-svg-dom-1.9.1.jar;./drivenbymoss-libs/batik-svggen-1.9.1.jar;./drivenbymoss-libs/batik-transcoder-1.9.1.jar;./drivenbymoss-libs/batik-util-1.9.1.jar;./drivenbymoss-libs/batik-xml-1.9.1.jar;./drivenbymoss-libs/commons-io-1.3.1.jar;./drivenbymoss-libs/commons-lang3-3.2.1.jar;./drivenbymoss-libs/commons-logging-1.0.4.jar;./drivenbymoss-libs/coremidi4j-1.1.jar;./drivenbymoss-libs/DrivenByMoss4Reaper-2.10.jar;./drivenbymoss-libs/javaosc-core-0.4.jar;./drivenbymoss-libs/jna-4.0.0.jar;./drivenbymoss-libs/jython-2.7.0.jar;./drivenbymoss-libs/libusb4java-1.2.0-linux-arm.jar;./drivenbymoss-libs/libusb4java-1.2.0-linux-x86.jar;./drivenbymoss-libs/libusb4java-1.2.0-linux-x86_64.jar;./drivenbymoss-libs/libusb4java-1.2.0-osx-x86.jar;./drivenbymoss-libs/libusb4java-1.2.0-osx-x86_64.jar;./drivenbymoss-libs/libusb4java-1.2.0-windows-x86.jar;./drivenbymoss-libs/libusb4java-1.2.0-windows-x86_64.jar;./drivenbymoss-libs/purejavahidapi-0.0.11-javadoc.jar;./drivenbymoss-libs/purejavahidapi-0.0.11-sources.jar;./drivenbymoss-libs/purejavahidapi-0.0.11.jar;./drivenbymoss-libs/rhino-1.7.7.jar;./drivenbymoss-libs/serializer-2.7.2.jar;./drivenbymoss-libs/usb4java-1.2.0.jar;./drivenbymoss-libs/xalan-2.7.2.jar;./drivenbymoss-libs/xml-apis-1.3.04.jar;./drivenbymoss-libs/xml-apis-ext-1.3.04.jar;./drivenbymoss-libs/xmlgraphics-commons-2.2.jar;./drivenbymoss-libs/inieditor-r6.jar";
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
		ReaScriptError("ERROR: Could not start Java Virtual Machine.");
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

	jclass transformatorAppClass = env->FindClass("de/mossgrabers/transformator/TransformatorApplication");
	const int result = env->RegisterNatives(transformatorAppClass, methods, 5);
	if (result == 0)
		return;
	jthrowable ex = env->ExceptionOccurred();
	if (ex)
	{
		jboolean isCopy = false;
		jmethodID toString = env->GetMethodID(env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
		jstring s = static_cast<jstring>(env->CallObjectMethod(ex, toString));
		const char* utf = env->GetStringUTFChars(s, &isCopy);
		std::stringstream stream;
		stream << "ERROR: Could not register native functions" << utf;
		env->ExceptionClear();
		ReaScriptError(stream.str().c_str());
	}
	else
		ReaScriptError("ERROR: Could not register native functions");
}


/**
 * Call the main method in the main class of the JVM.
 */
void JvmManager::StartApp()
{
	jclass transformatorClass = env->FindClass("de/mossgrabers/transformator/Transformator");
	if (transformatorClass == nullptr)
		return;
	// Call main start method
	jmethodID methodID = env->GetStaticMethodID(transformatorClass, "main", "([Ljava/lang/String;)V");
	if (methodID == nullptr)
		return;
	const char*iniPath = GetResourcePath();
	jobjectArray applicationArgs = env->NewObjectArray(1, env->FindClass("java/lang/String"), nullptr);
	env->SetObjectArrayElement(applicationArgs, 0, env->NewStringUTF(iniPath));
	env->CallStaticVoidMethod(transformatorClass, methodID, applicationArgs);
}
