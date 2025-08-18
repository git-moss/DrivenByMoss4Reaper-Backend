// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "stdafx.h"

#define REAPERAPI_IMPLEMENT

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <numeric>
#include <set>
#include <thread>
#include <unordered_map>
#include <wdltypes.h>
#include <lineparse.h>

#include "resource.h"

#include "CodeAnalysis.h"
#include "DrivenByMossSurface.h"
#include "LocalMidiEventDispatcher.h"
#include "MidiProcessingStructures.h"
#include "ReaDebug.h"
#include "StringUtils.h"

midi_Input* (*GetMidiInput)(int idx);
midi_Output* (*GetMidiOutput)(int idx);

// The global extension variables required to bridge from C to C++,
// static keyword restricts the visibility of a function to the file
REAPER_PLUGIN_HINSTANCE pluginInstanceHandle = nullptr;
gaccel_register_t openDBMConfigureWindowAccel = { {0,0,0}, "DrivenByMoss: Open the configuration window." };
gaccel_register_t openDBMProjectWindowAccel = { {0,0,0}, "DrivenByMoss: Open the project settings window." };
gaccel_register_t openDBMParameterWindowAccel = { {0,0,0}, "DrivenByMoss: Open the parameter settings window." };
gaccel_register_t restartControllersAccel = { {0,0,0}, "DrivenByMoss: Restart all controllers." };

// Access to Java side via JNI
std::unique_ptr <JvmManager> jvmManager;

// MIDI port handling
std::set<int> activeMidiOutputs;
std::set<int> activeMidiInputs;

// The audio hook for MIDI communication
audio_hook_register_t audioHook;

// Atomic shared snapshot (lock-free access)
using DeviceMap = std::unordered_map<int, DeviceNoteData>;
std::shared_ptr<DeviceMap> deviceDataSnapshot;
std::mutex deviceDataMutex; // only needed for writes, not reads

// Java to internal Reaper
LocalMidiEventDispatcher localMidiEventDispatcher{};

// Defined in DrivenByMossSurface.cpp
extern DrivenByMossSurface* surfaceInstance;


/**
 * Java callback for an OSC style command to be executed in Reaper without a parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 */
static void ProcessNoArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a string parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The string value
 */
static void ProcessStringArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jstring value)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* val = env->GetStringUTFChars(value, nullptr);
	if (val == nullptr)
	{
		env->ReleaseStringUTFChars(processor, proc);
		return;
	}
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	std::string valueString(val);
	surfaceInstance->GetOscParser().Process(procstr, path, valueString);
	env->ReleaseStringUTFChars(value, val);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with several string parameters.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param values    The string values
 */
static void ProcessStringArgsCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jobjectArray values)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;

	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;

	const int stringCount = env->GetArrayLength(values);
	std::vector<std::string> parameters;
	for (int i = 0; i < stringCount; i++)
	{
		jobject objectValue = env->GetObjectArrayElement(values, i);
		// jobject is not polymorphic
		DISABLE_WARNING_NO_STATIC_DOWNCAST
		jstring value = static_cast<jstring>(objectValue);
		const char* val = env->GetStringUTFChars(value, nullptr);
		if (val == nullptr)
			continue;
		parameters.push_back(val);

		env->ReleaseStringUTFChars(value, val);
	}

	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);

	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);

	surfaceInstance->GetOscParser().Process(procstr, path, parameters);

	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with an integer parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The integer value
 */
static void ProcessIntArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jint value)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path, gsl::narrow_cast<int> (value));
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback for an OSC style command to be executed in Reaper with a double parameter.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to execute the command
 * @param command   The command to execute
 * @param value     The double value
 */
static void ProcessDoubleArgCPP(JNIEnv* env, jobject object, jstring processor, jstring command, jdouble value)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	const char* cmd = command == nullptr ? nullptr : env->GetStringUTFChars(command, nullptr);
	std::string procstr(proc);
	std::string path(cmd == nullptr ? "" : cmd);
	surfaceInstance->GetOscParser().Process(procstr, path, value);
	env->ReleaseStringUTFChars(processor, proc);
	if (cmd != nullptr)
		env->ReleaseStringUTFChars(command, cmd);
}


/**
 * Java callback to dis-/enable updates for a specific processor.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to delay
 * @param enable    True to enable updates for the processor
 */
static void EnableUpdatesCPP(JNIEnv* env, jobject object, jstring processor, jboolean enable)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surfaceInstance->GetDataCollector().EnableUpdate(procstr, enable);
	env->ReleaseStringUTFChars(processor, proc);
}


/**
 * Java callback to delay updates for a specific processor. Use to prevent that Reaper sends old
 * values before the latest ones are applied.
 *
 * @param env       The JNI environment
 * @param object    The JNI object
 * @param processor The processor to delay
 */
static void DelayUpdatesCPP(JNIEnv* env, jobject object, jstring processor)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;
	const char* proc = env->GetStringUTFChars(processor, nullptr);
	if (proc == nullptr)
		return;
	std::string procstr(proc);
	surfaceInstance->GetDataCollector().DelayUpdate(procstr);
	env->ReleaseStringUTFChars(processor, proc);
}


/**
 * Send a MIDI message to Reaper.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The index of the MIDI input device
 * @param status   MIDI status byte
 * @param data1    MIDI data byte 1
 * @param data2    MIDI data byte 2
 */
static void ProcessMidiArgCPP(const JNIEnv* env, jobject object, jint deviceID, jint status, jint data1, jint data2)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;

	MIDI_event_t event;
	event.frame_offset = 0;
	event.size = 3;
	event.midi_message[0] = gsl::narrow_cast<unsigned char>(status);
	event.midi_message[1] = gsl::narrow_cast<unsigned char>(data1);
	event.midi_message[2] = gsl::narrow_cast<unsigned char>(data2);
	localMidiEventDispatcher.Push(deviceID, event);
}


/**
 * Java callback to get all available MIDI inputs.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @return The TreeMap with all available MIDI inputs. The key holds the ID and the value the name.
 */
static jobject GetMidiInputsCPP(JNIEnv* env, jobject object)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return nullptr;

	jobject treeMap = surfaceInstance->jvmManager->CreateTreeMap(*env);
	const int numMidiInputs = GetNumMIDIInputs();
	for (int deviceID = 0; deviceID < numMidiInputs; deviceID++)
	{
		// Only add MIDI inputs which are open
		const midi_Input* midiin = GetMidiInput(deviceID);
		if (!midiin)
			continue;

		char buf[512];
		DISABLE_WARNING_ARRAY_POINTER_DECAY
		if (GetMIDIInputName(deviceID, buf, sizeof(buf)))
			surfaceInstance->jvmManager->AddToTreeMap(*env, treeMap, deviceID, buf);
	}

	return treeMap;
}


/**
 * Java callback to get all available MIDI outputs.
 *
 * @param env     The JNI environment
 * @param object  The JNI object
 * @return The TreeMap with all available MIDI outputs. The key holds the ID and the value the name.
 */
static jobject GetMidiOutputsCPP(JNIEnv* env, jobject object)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return nullptr;

	jobject treeMap = surfaceInstance->jvmManager->CreateTreeMap(*env);
	const int numMidiOutputs = GetNumMIDIOutputs();
	for (int deviceID = 0; deviceID < numMidiOutputs; deviceID++)
	{
		// Only add MIDI inputs which are open
		const midi_Output* midiout = GetMidiOutput(deviceID);
		if (!midiout)
			continue;

		char buf[512];
		DISABLE_WARNING_ARRAY_POINTER_DECAY
		if (GetMIDIOutputName(deviceID, buf, sizeof(buf)))
			surfaceInstance->jvmManager->AddToTreeMap(*env, treeMap, deviceID, buf);
	}

	return treeMap;
}


/**
 * Java callback to open a MIDI input.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The ID of the MIDI input to open
 * @return True if the input was opened successfully
 */
static jboolean OpenMidiInputCPP(JNIEnv* env, jobject object, jint deviceID)
{
	const int id = gsl::narrow_cast<int>(deviceID);
	if (id >= 0 && id < GetNumMIDIInputs())
	{
		activeMidiInputs.insert(deviceID);
		return JNI_TRUE;
	}
	return JNI_FALSE;
}


/**
 * Java callback to open a MIDI output.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The ID of the MIDI output to open
 * @return True if the output was opened successfully
 */
static jboolean OpenMidiOutputCPP(JNIEnv* env, jobject object, jint deviceID)
{
	const int id = gsl::narrow_cast<int>(deviceID);
	if (id >= 0 && id < GetNumMIDIOutputs())
	{
		activeMidiOutputs.insert(deviceID);
		return JNI_TRUE;
	}
	return JNI_FALSE;
}


/**
 * Java callback to close a MIDI input.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The ID of the MIDI input to close
 */
static void CloseMidiInputCPP(JNIEnv* env, jobject object, jint deviceID) noexcept
{
	activeMidiInputs.erase(deviceID);
}


/**
 * Java callback to close a MIDI output.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The ID of the MIDI output to close
 */
static void CloseMidiOutputCPP(JNIEnv* env, jobject object, jint deviceID) noexcept
{
	activeMidiOutputs.erase(deviceID);
}


/**
 * Java callback to send a MIDI message to a MIDI output port.
 *
 * @param env      The JNI environment
 * @param object   The JNI object
 * @param deviceID The ID of the MIDI output to send the message to
 * @param data     The data of the message to send
 */
static void SendMidiDataCPP(JNIEnv* env, jobject object, jint deviceID, jbyteArray data)
{
	if (env == nullptr || surfaceInstance == nullptr)
		return;

	const jsize length = env->GetArrayLength(data);
	if (length <= 0)
		return;

	// Fast path: get bytes into a temp buffer on the JNI thread
	jboolean isCopy;
	jbyte* source = env->GetByteArrayElements(data, &isCopy);
	if (source == nullptr)
		return;

	if (length <= 3)
	{
		// ---------- 1‑3 bytes ----------
		const gsl::span<jbyte> spanSource(source, length);
		std::unique_ptr<Midi3> m = std::make_unique<Midi3>();
		m->deviceId = gsl::narrow_cast<uint32_t>(deviceID);
		m->status = gsl::narrow_cast<uint8_t>(source[0]);
		m->data1 = length > 1 ? gsl::narrow_cast<uint8_t>(gsl::at(spanSource, 1)) : 0;
		m->data2 = length > 2 ? gsl::narrow_cast<uint8_t>(gsl::at(spanSource, 2)) : 0;
		surfaceInstance->outgoingMidiQueue3.push(std::move(m));
	}
	else if (length <= kSyx1k_Max)
	{
		// ---------- ≤ 1 024 bytes ----------
		std::unique_ptr<MidiSyx1k> m = std::make_unique<MidiSyx1k>();
		m->deviceId = gsl::narrow_cast<uint32_t>(deviceID);
		m->size = gsl::narrow_cast<uint32_t>(length);
		std::memcpy(m->data, source, length);
		surfaceInstance->outgoingMidiQueue1k.push(std::move(m));
	}
	else if (length <= kSyx64k_Max)
	{
		// ---------- ≤ 65 536 bytes ----------
		std::unique_ptr<MidiSyx64k> m = std::make_unique<MidiSyx64k>();
		m->deviceId = gsl::narrow_cast<uint32_t>(deviceID);
		m->size = gsl::narrow_cast<uint32_t>(length);
		std::memcpy(m->data, source, length);
		surfaceInstance->outgoingMidiQueue64k.push(std::move(m));
	}
	else
	{
		ReaDebug::Log("DrivenByMoss: Attempt to send MIDI message greater 64k.\n");
		return;
	}

	env->ReleaseByteArrayElements(data, source, JNI_ABORT);
}


/**
 * Convert hex string to vector of unsigned char.
 * 
 * @param hexStr The hex codes to parse, needs to be an equal number of characters
 * @return The parsed pair-characters as byte numbers
 */
static std::vector<unsigned char> ParseHexFilter(const std::string& hexStr)
{
	std::vector<unsigned char> result;
	for (size_t i = 0; i < hexStr.length(); i += 2)
	{
		std::string byteStr = hexStr.substr(i, 2);
		unsigned int byteVal;
		std::stringstream ss;
		ss << std::hex << byteStr;
		ss >> byteVal;
		result.push_back(static_cast<unsigned char>(byteVal));
	}
	return result;
}


/**
 * Set the MIDI filters for a note input.
 *
 * @param env            The JNI environment
 * @param object         The JNI object
 * @param deviceID       The ID of the MIDI input to which the note input belongs
 * @param noteInputIndex The index of the note input to which the filters belong
 */
static void SetFiltersCPP(JNIEnv* env, jobject object, jint deviceID, jint noteInputIndex, jobjectArray filters)
{
	if (env == nullptr || noteInputIndex < 0 || noteInputIndex >= MAX_NOTE_INPUTS)
		return;

	const jsize count = env->GetArrayLength(filters);
	FilterSet parsed;

	for (jsize i = 0; i < count; ++i)
	{
		jstring jStr = static_cast<jstring>(env->GetObjectArrayElement(filters, i));
		const char* raw = env->GetStringUTFChars(jStr, nullptr);
		std::string hex(raw);
		env->ReleaseStringUTFChars(jStr, raw);
		env->DeleteLocalRef(jStr);

		// This parser expects 2 or 4 charcters!
		hex.erase(std::remove_if(hex.begin(), hex.end(), ::isspace), hex.end());
		if (hex.length() == 2 || hex.length() == 4)
			parsed.push_back(ParseHexFilter(hex));
	}

	const std::lock_guard<std::mutex> lock(deviceDataMutex);
	DISABLE_WARNING_PREFER_LOCAL_OBJECTS
	std::shared_ptr<DeviceMap> current = std::atomic_load(&deviceDataSnapshot);
	if (!current)
		current = std::make_shared<DeviceMap>();
	std::shared_ptr<DeviceMap> updated = std::make_shared<DeviceMap>(*current);

	DeviceNoteData& deviceData = (*updated)[deviceID];
	deviceData.noteInputs[noteInputIndex].filters = std::move(parsed);
	deviceData.BuildFilterLookup();

	std::atomic_store(&deviceDataSnapshot, updated);
}


/**
 * Safe copier from jintArray to std::array<int, 128>.
 */
inline static bool CopyJIntArray128(JNIEnv* env, jintArray arr, std::array<int, 128>& out)
{
	if (!env || !arr)
		return false;
	const jsize len = env->GetArrayLength(arr);
	if (len != 128)
		return false;

	DISABLE_WARNING_REINTERPRET_CAST
	// False positive
	DISABLE_WARNING_IDENTICAL_SOURCE_DEST
	env->GetIntArrayRegion(arr, 0, 128, reinterpret_cast<jint*>(out.data()));
	return true;
}


/**
 * Set the MIDI key translation table for a note input.
 *
 * @param env            The JNI environment
 * @param object         The JNI object
 * @param deviceID       The ID of the MIDI input to which the note input belongs
 * @param noteInputIndex The index of the note input to which the filters belong
 * @param table          The translation table which must contain 128 elements
 */
static void SetKeyTranslationTableCPP(JNIEnv* env, jobject object, jint deviceID, jint noteInputIndex, jintArray table)
{
	if (env == nullptr || table == nullptr || noteInputIndex < 0 || noteInputIndex >= MAX_NOTE_INPUTS)
		return;

	const jsize len = env->GetArrayLength(table);
	if (len != 128)
		return;

	std::array<int, 128> parsed;
	if (!CopyJIntArray128(env, table, parsed))
		return;

	const std::lock_guard<std::mutex> lock(deviceDataMutex);
	std::shared_ptr<DeviceMap> current = std::atomic_load(&deviceDataSnapshot);
	if (!current)
		current = std::make_shared<DeviceMap>();
	std::shared_ptr<DeviceMap> updated = std::make_shared<DeviceMap>(*current);

	DeviceNoteData& deviceData = (*updated)[deviceID];
	NoteInputData& noteData = deviceData.noteInputs[noteInputIndex];
	noteData.keyTable = parsed;
	deviceData.BuildKeyLookup();

	std::atomic_store(&deviceDataSnapshot, updated);
}


/**
 * Set the MIDI velocity translation table for a note input.
 *
 * @param env            The JNI environment
 * @param object         The JNI object
 * @param deviceID       The ID of the MIDI input to which the note input belongs
 * @param noteInputIndex The index of the note input to which the filters belong
 * @param table          The translation table which must contain 128 elements
 */
static void SetVelocityTranslationTableCPP(JNIEnv* env, jobject object, jint deviceID, jint noteInputIndex, jintArray table)
{
	if (env == nullptr || table == nullptr || noteInputIndex < 0 || noteInputIndex >= MAX_NOTE_INPUTS)
		return;

	const jsize len = env->GetArrayLength(table);
	if (len != 128)
		return;

	std::array<int, 128> parsed;
	if (!CopyJIntArray128(env, table, parsed))
		return;

	std::shared_ptr<DeviceMap> current = std::atomic_load(&deviceDataSnapshot);
	if (!current)
		current = std::make_shared<DeviceMap>();
	std::shared_ptr<DeviceMap> updated = std::make_shared<DeviceMap>(*current);

	DeviceNoteData& deviceData = (*updated)[deviceID];
	NoteInputData& noteData = deviceData.noteInputs[noteInputIndex];
	noteData.velocityTable = parsed;
	deviceData.BuildKeyLookup();

	std::atomic_store(&deviceDataSnapshot, updated);
}


/**
 * Callback for custom (keyboard) actions.
 * 
 * @param command The ID of the command to execute
 * @param flag    Execution flag, not used
 */
static bool HookCommandProc(int command, int flag)
{
	if (!jvmManager || !jvmManager->IsRunning())
		return false;

	if (openDBMConfigureWindowAccel.accel.cmd != 0 && openDBMConfigureWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayWindow();
		return true;
	}
	if (openDBMProjectWindowAccel.accel.cmd != 0 && openDBMProjectWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayProjectWindow();
		return true;
	}
	if (openDBMParameterWindowAccel.accel.cmd != 0 && openDBMParameterWindowAccel.accel.cmd == command)
	{
		jvmManager->DisplayParameterWindow();
		return true;
	}
	if (restartControllersAccel.accel.cmd != 0 && restartControllersAccel.accel.cmd == command)
	{
		jvmManager->RestartControllers();
		return true;
	}
	return false;
}


/**
 * (Windows) message callback of the configuration dialog.
 * 
 * @param hwndDlg The handle of the dialog window
 * @param uMsg    The message to handle
 * @param wParam  The int parameter
 * @param lParam  The long parameter
 */
static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		std::string path = jvmManager ? jvmManager->GetJavaHomePath() : "Filled when the dialog is opened again...";
#ifdef _WIN32
		SetDlgItemText(hwndDlg, IDC_JAVA_HOME, stringToWs(path).c_str());
#else
		SetDlgItemText(hwndDlg, IDC_JAVA_HOME, path.c_str());
#endif
		if (jvmManager)
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_REOPEN_INFO), SW_HIDE);
		}
		else
		{
			ShowWindow(GetDlgItem(hwndDlg, IDC_JAVA_HOME_LBL), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_JAVA_HOME), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_BUTTON_CONFIGURE), SW_HIDE);
		}
	}
	break;

	// Display the configuration window
	case WM_COMMAND:
	{
		DISABLE_WARNING_NO_C_STYLE_CONVERSION
		const WORD value = LOWORD(wParam);
		switch (value)
		{
		case IDC_BUTTON_CONFIGURE:
			if (jvmManager && jvmManager->IsRunning())
				jvmManager->DisplayWindow();
			break;
		default:
			// Ignore the rest
			break;
		}
	}
	break;

	default:
		// Ignore the rest
		break;
	}
	return 0;
}


/**
 * Callback function for Reaper to create an instance of the extension.
 * 
 * @param type_string  Unused
 * @param configString Unused
 * @param errStats     Unused
 */
IReaperControlSurface* createFunc(const char* type_string, const char* configString, int* errStats)
{
	if (!ENABLE_EXTENSION)
		return nullptr;

	// Note: If the setup dialog is closed with OK, the current surfaceInstance will be destructed but
	// we cannot create a new JVM, since this is only possible once!
	if (ENABLE_JAVA && jvmManager == nullptr)
	{
		jvmManager = std::make_unique<JvmManager>(DEBUG_JAVA);
		if (jvmManager == nullptr)
			return nullptr;

		// Satisfying C API
		DISABLE_WARNING_REINTERPRET_CAST
		void* functions[18] = {
			reinterpret_cast<void*>(&ProcessNoArgCPP),
			reinterpret_cast<void*>(&ProcessStringArgCPP),
			reinterpret_cast<void*>(&ProcessStringArgsCPP),
			reinterpret_cast<void*>(&ProcessIntArgCPP),
			reinterpret_cast<void*>(&ProcessDoubleArgCPP),
			reinterpret_cast<void*>(&EnableUpdatesCPP),
			reinterpret_cast<void*>(&DelayUpdatesCPP),
			reinterpret_cast<void*>(&ProcessMidiArgCPP),
			reinterpret_cast<void*>(&GetMidiInputsCPP),
			reinterpret_cast<void*>(&GetMidiOutputsCPP),
			reinterpret_cast<void*>(&OpenMidiInputCPP),
			reinterpret_cast<void*>(&OpenMidiOutputCPP),
			reinterpret_cast<void*>(&CloseMidiInputCPP),
			reinterpret_cast<void*>(&CloseMidiOutputCPP),
			reinterpret_cast<void*>(&SendMidiDataCPP),
			reinterpret_cast<void*>(&SetFiltersCPP),
			reinterpret_cast<void*>(&SetKeyTranslationTableCPP),
			reinterpret_cast<void*>(&SetVelocityTranslationTableCPP)
		};

		jvmManager->Init(functions);
	}

	ReaDebug::Log("DrivenByMoss: Creating surface.\n");

	// Note: delete is called from Reaper on shutdown, no need to do it ourselves
	DISABLE_WARNING_DONT_USE_NEW
	surfaceInstance = new DrivenByMossSurface(jvmManager, GetMidiOutput);

	ReaDebug::Log("DrivenByMoss: Surface created.\n");

	return surfaceInstance;
}

// Callback function for Reaper to create the configuration dialog of the extension
static HWND configFunc(const char* type_string, HWND parent, const char* initConfigString) noexcept
{
	// No way to prevent the LPARAM cast
	DISABLE_WARNING_REINTERPRET_CAST
	return CreateDialogParam(pluginInstanceHandle, MAKEINTRESOURCE(IDD_SURFACEEDIT_DRIVENBYMOSS), parent, dlgProc, reinterpret_cast<LPARAM>(initConfigString));
}

// Description for DrivenByMoss surfaceInstance extension
static reaper_csurf_reg_t drivenbymoss_reg =
{
	"DrivenByMoss4Reaper",
	"DrivenByMoss4Reaper",
	createFunc,
	configFunc,
};


/**
 * Called for all extension sub-tags of the project file to parse additional parameters which are specific to an extension.
 * 
 * @param line   The line to parse
 * @param ctx    The project context (not used)
 * @param isUndo Is this called due to an undo operation? (not used)
 * @param reg    Pointer to the registered extension (not used)
 */
static bool ProcessExtensionLine(const char* line, ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Processing project parameters.\n");

	if (ctx == nullptr)
		return false;

	if (!jvmManager || !jvmManager->IsRunning())
		return false;

	// Parse the line and check if it is valid and belongs to this extension
	LineParser lp(false);
	if (lp.parse(line) || lp.getnumtokens() < 1 || strcmp(lp.gettoken_str(0), "<DRIVEN_BY_MOSS") != 0)
	{
		ReaDebug::Log("DrivenByMoss: Processing project parameters done - none found.\n");
		return false;
	}

	constexpr int LENGTH = 8000;
	std::string data(LENGTH, 0);
	char* dataPointer = &*data.begin();
	while (true)
	{
		if (ctx->GetLine(dataPointer, LENGTH) || lp.parse(dataPointer))
			break;

		const char* token = lp.gettoken_str(0);
		if (token != nullptr)
		{
			std::string tokenStr = token;
			if (tokenStr.at(0) == '>')
				break;
		}

		jvmManager->SetFormattedDocumentSettings(data);
	}

	ReaDebug::Log("DrivenByMoss: Processing project parameters done.\n");
	return true;
}


/**
 * Callback for storing additional parameters for the registered extension.
 *
 * @param ctx    The project context (not used)
 * @param isUndo Is this called due to an undo operation? (not used)
 * @param reg    Pointer to the registered extension (not used)
 */
static void SaveExtensionConfig(ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Saving project settings.\n");

	if (ctx == nullptr)
		return;

	if (!jvmManager || !jvmManager->IsRunning())
		return;

	std::string line = jvmManager->GetFormattedDocumentSettings();

	ctx->AddLine("<DRIVEN_BY_MOSS");
	ctx->AddLine("%s", line.c_str());
	ctx->AddLine(">");

	ReaDebug::Log("DrivenByMoss: Project settings saved.\n");
}


/**
 * Callback for loading additional parameters for the registered extension.
 *
 * @param ctx    The project context (not used)
 * @param isUndo Is this called due to an undo operation? (not used)
 * @param reg    Pointer to the registered extension (not used)
 */
static void BeginLoadProjectState(bool isUndo, struct project_config_extension_t* reg)
{
	ReaDebug::Log("DrivenByMoss: Loading project settings.\n");

	// Called on project load/undo before any (possible) ProcessExtensionLine. NULL is OK too
	// also called on "new project" (wont be followed by ProcessExtensionLine calls in that case)
	// Defaults could be set here but are already set by the controller instances
	if (jvmManager && jvmManager->IsRunning())
		jvmManager->SetDefaultDocumentSettings();

	ReaDebug::Log("DrivenByMoss: Project settings loaded.\n");
}

// Structure for registering the project notification extension
project_config_extension_t pcreg =
{
  ProcessExtensionLine,
  SaveExtensionConfig,
  BeginLoadProjectState,
  nullptr,
};


/**
 * Matches the incoming MIDI event against the registered note input filters.
 * 
 * @param deviceID The ID of the device to which to match the event
 * @param deviceID The event to match/filter
 */
inline static bool ProcessMidiEvents(int deviceID, MIDI_event_t* event)
{
	const unsigned char data1 = event->size > 1 ? event->midi_message[1] : 0;
	if (data1 >= 128)
		return false;

	auto snapshot = std::atomic_load(&deviceDataSnapshot);
	if (!snapshot)
		return false;

	auto deviceIt = snapshot->find(deviceID);
	if (deviceIt == snapshot->end())
		return false;

	const auto& deviceData = deviceIt->second;

	// Do not use gsl:at for performance reasons!
	DISABLE_WARNING_USE_GSL_AT
	DISABLE_WARNING_ACCESS_ARRAYS_WITH_CONST

	const unsigned char status = event->midi_message[0];
	const int statusType = status & 0xF0;
	const bool isNote = statusType == 0x90 || statusType == 0x80;

	for (int noteIdx = 0; noteIdx < MAX_NOTE_INPUTS; ++noteIdx)
	{
		// Note: to be 100% correct this would require the creation of a new MIDI event since
		// theoretically multiple note inputs could be present and events could be modified differently
		// If this becomes a use-case it would need to be implemented with a pre-allocated pool or ring 
		// buffer of MIDI_event_t

		if (deviceData.filterMatch[noteIdx][status][data1])
		{
			if (isNote && deviceData.filterMatch[noteIdx][status][data1])
			{
				if (deviceData.keyLookup[noteIdx][data1] < 0)
					continue;
					
				event->midi_message[1] = deviceData.keyLookup[noteIdx][data1];

				const unsigned char data2 = event->size > 2 ? event->midi_message[2] : 0;

				if (deviceData.velocityLookup[noteIdx][data2] < 0)
					continue;

				if (data2 >= 128)
					return false;

				event->midi_message[2] = deviceData.velocityLookup[noteIdx][data2];
			}

			return true;
		}
	}

	return false;
}


/**
 * Audio hook. Reads from registered MIDI inputs and queues the events to be sent to Java as well
 * as matching them against the registered note input filters and sends the matches to Reaper.
 * Adds additional MIDI events intended for Reaper.
 * The method is called before and after the update of the audio buffer
 * 
 * @param isPost True if the call is after the update of the audio buffer
 * @param len    The length of the buffer (not used)
 * @param srate  The sample rate (not used)
 * @param reg    Pointer to the registered audio hook structure (not used)
 */
static void OnAudioBuffer(bool isPost, int len, double srate, struct audio_hook_register_t* reg)
{
	if (jvmManager == nullptr || surfaceInstance == nullptr || isPost)
		return;

	for (const auto& deviceID : activeMidiInputs)
	{
		midi_Input* midiin = GetMidiInput(deviceID);
		if (!midiin)
			continue;

		MIDI_eventlist* list = midiin->GetReadBuf();
		if (!list)
			continue;

		int position = 0;
		int nextPosition = 0;
		MIDI_event_t* event;
		// Copy the events out of the audio thread to be sent to the Java side
		while ((event = list->EnumItems(&nextPosition)) != nullptr)
		{
			const int size = event->size;
			if (size == 0)
				continue;

			if (size > 3)
			{
				if (size < 1024)
					surfaceInstance->EnqueueSysex1k(deviceID, event->midi_message, event->size);
				else
					surfaceInstance->EnqueueSysex64k(deviceID, event->midi_message, event->size);
				continue;
			}

			const uint8_t status = event->midi_message[0];
			// Ignore active sensing
			if (status == 0 || status == 0xFE)
				continue;

			const uint8_t data1 = (size > 1) ? event->midi_message[1] : 0;
			const uint8_t data2 = (size > 2) ? event->midi_message[2] : 0;
			surfaceInstance->EnqueueMidi3(deviceID, status, data1, data2);
			// Apply note input filters
			if (!ProcessMidiEvents(deviceID, event))
			{
				list->DeleteItem(position);
				nextPosition = position;
			}
			else
				position = nextPosition;
		}

		// Add events to be sent to Reaper
		localMidiEventDispatcher.ProcessDeviceQueue(deviceID, list);
	}
}


// Must be extern to be exported from the DLL
extern "C"
{
	// Defines the entry point for the DLL application.
	// It is always executed after Reaper loaded it, even if the extension is not added in the configuration dialog!
	REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, const reaper_plugin_info_t* rec)
	{
		if (!ENABLE_EXTENSION)
			return 0;

		// On shutdown
		if (rec == nullptr)
		{
			if (jvmManager)
			{
				jvmManager->SetCleanShutdown();
				jvmManager.reset();
			}
			return 0;
		}

		// On startup...

		pluginInstanceHandle = hInstance;
		ReaperUtils::mainWindowHandle = rec->hwnd_main;
		deviceDataSnapshot = std::make_shared<DeviceMap>();

		if (rec->caller_version != REAPER_PLUGIN_VERSION || rec->GetFunc == nullptr)
			return 0;

		// False positive, null check above is not detected
		DISABLE_WARNING_DANGLING_POINTER
		REAPERAPI_LoadAPI(rec->GetFunc);

		// Register extension
		const int result = rec->Register("csurf", &drivenbymoss_reg);
		if (!result)
		{
			ReaDebug() << "Could not instantiate DrivenByMoss surfaceInstance extension.";
			return 0;
		}

		// Register project notifications
		rec->Register("projectconfig", &pcreg);

		// Register actions
		openDBMConfigureWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_WINDOW_ACTION");
		if (!openDBMConfigureWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open configuration window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMConfigureWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open window action.";
			return 0;
		}

		openDBMProjectWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_PROJECT_WINDOW_ACTION");
		if (!openDBMProjectWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open project window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMProjectWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open project window action.";
			return 0;
		}

		openDBMParameterWindowAccel.accel.cmd = rec->Register("command_id", (void*)"DBM_OPEN_PARAMETER_WINDOW_ACTION");
		if (!openDBMParameterWindowAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss open parameter mapping window action.";
			return 0;
		}
		if (!rec->Register("gaccel", &openDBMParameterWindowAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss open parameter window action.";
			return 0;
		}

		restartControllersAccel.accel.cmd = rec->Register("command_id", (void*)"RESTART_CONTROLLERS_ACTION");
		if (!restartControllersAccel.accel.cmd)
		{
			ReaDebug() << "Could not register ID for DrivenByMoss restart controllers action.";
			return 0;
		}
		if (!rec->Register("gaccel", &restartControllersAccel))
		{
			ReaDebug() << "Could not register DrivenByMoss restart controllers action.";
			return 0;
		}

		if (!rec->Register("hookcommand", (void*)HookCommandProc))
		{
			ReaDebug() << "Could not register DrivenByMoss action callback.";
			return 0;
		}

		// Make undocumented GetMidiInput and GetMidiOutput functions available
		GetMidiInput = (midi_Input * (*)(int))rec->GetFunc("GetMidiInput");
		if (!GetMidiInput)
		{
			ReaDebug() << "GetMidiInput is not available.";
			return 0;
		}
		GetMidiOutput = (midi_Output * (*)(int))rec->GetFunc("GetMidiOutput");
		if (!GetMidiOutput)
		{
			ReaDebug() << "GetMidiOutput is not available.";
			return 0;
		}

		// Register audio hook
		audioHook.userdata1 = nullptr;
		audioHook.userdata2 = nullptr;
		audioHook.input_nch = 0;
		audioHook.output_nch = 0;
		audioHook.GetBuffer = nullptr;
		audioHook.OnAudioBuffer = OnAudioBuffer;
		const int hookResult = Audio_RegHardwareHook(true, &audioHook);
		if (!hookResult)
		{
			ReaDebug() << "AudioHook could not be registered!.";
			return 0;
		}

		return 1;
	}
};


#ifndef _WIN32 // MAC resources
#include "swell-dlggen.h"
#include "res.rc_mac_dlg"
#include "dllmain.h"
#undef BEGIN
#undef END
#endif
