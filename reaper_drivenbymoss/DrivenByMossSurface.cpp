// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt


#include "ReaDebug.h"
#include "DrivenByMossSurface.h"

// Singleton, deleted from Reaper
DrivenByMossSurface* surfaceInstance = nullptr;


/**
 * Constructor.
 */
DISABLE_WARNING_NO_REF_TO_UNIQUE_PTR
DrivenByMossSurface::DrivenByMossSurface(std::unique_ptr<JvmManager>& aJvmManager, midi_Output* (*aGetMidiOutput)(int idx)) : jvmManager(aJvmManager), GetMidiOutput(aGetMidiOutput), model(functionExecutor)
{
	ReaDebug::setModel(&model);
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	this->isShutdown = true;

	// Null global variables
	ReaDebug::setModel(nullptr);
	surfaceInstance = nullptr;
}


const char* DrivenByMossSurface::GetTypeString() noexcept
{
	return "DrivenByMoss4Reaper";
}


const char* DrivenByMossSurface::GetDescString() noexcept
{
	return "DrivenByMoss4Reaper - Supports lot's of surfaces...";
}


const char* DrivenByMossSurface::GetConfigString() noexcept
{
	// String must not be empty or otherwise the surface is not instantiated on startup
	return "empty";
}


/**
 * The control surface callback function for updating the device. Called 30x/sec or so.
 */
void DrivenByMossSurface::Run()
{
	if (this->jvmManager == nullptr || !this->jvmManager->IsRunning() || this->isShutdown)
		return;

	// Infrastructure needs to startup here to ensure that the Reaper audio layer is up 
	// and running otherwise there might be a deadlock on Macos
	if (!this->isInfrastructureUp)
	{
		if (Audio_IsRunning() == 0)
			return;
		const std::lock_guard<std::mutex> lock(this->startInfrastructureMutex);
		if (!this->isInfrastructureUp)
		{
			this->jvmManager->StartInfrastructure();
			this->isInfrastructureUp = true;
		}
	}

	try
	{
		this->SendMIDIEventsToJava();
		surfaceInstance->SendMIDIEventsToOutputs();
		this->functionExecutor.ExecuteFunctions();
	}
	catch (const std::exception& ex)
	{
		ReaDebug() << "Could not update device: " << ex.what();
	}
	catch (...)
	{
		ReaDebug() << "Could not update device.";
	}

	this->oscParser.GetActionProcessor().CheckActionSelection();

	// Only update each 2nd call (about 60ms)
	this->updateModel = !this->updateModel;
	if (!this->updateModel)
		return;

	std::string data = this->CollectData(this->model.ShouldDump());
	if (data.length() > 0)
		this->jvmManager.get()->UpdateModel(data);
}


void DrivenByMossSurface::SetTrackListChange() noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceVolume(MediaTrack* trackid, double volume) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfacePan(MediaTrack* trackid, double pan) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceMute(MediaTrack* trackid, bool mute) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSelected(MediaTrack* trackid, bool selected) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceSolo(MediaTrack* trackid, bool solo) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetSurfaceRecArm(MediaTrack* trackid, bool recarm) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetPlayState(bool play, bool pause, bool rec) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetRepeatState(bool rep) noexcept
{
	// Not used
}

void DrivenByMossSurface::SetTrackTitle(MediaTrack* trackid, const char* title) noexcept
{
	// Not used
}

bool DrivenByMossSurface::GetTouchState(MediaTrack* trackid, int isPan)
{
	if (trackid == GetMasterTrack(ReaperUtils::GetProject()))
		return isPan ? model.isMasterPanTouch : model.isMasterVolumeTouch;

	const int position = static_cast<int>(GetMediaTrackInfo_Value(trackid, "IP_TRACKNUMBER")) - 1;
	if (position < 0)
		return false;

	const std::unique_ptr<Track>& trackPtr = model.GetTrack(position);
	return isPan ? trackPtr->isPanTouch : trackPtr->isVolumeTouch;
}

void DrivenByMossSurface::SetAutoMode(int mode) noexcept
{
	// Not used
}


void DrivenByMossSurface::ResetCachedVolPanStates() noexcept
{
	// Not used
}


void DrivenByMossSurface::OnTrackSelection(MediaTrack* trackid) noexcept
{
	// Not used
}


void DrivenByMossSurface::SendMIDIEventsToJava()
{
	if (this->isShutdown)
		return;

	// Satisfy the C API
	DISABLE_WARNING_ARRAY_POINTER_DECAY

	// --- 3‑byte messages ----------------------------------------
	std::unique_ptr<Midi3> m3;
	while ((m3 = this->incomingMidiQueue3.pop()) != nullptr)
	{
		uint8_t raw[3] = { m3->status, m3->data1, m3->data2 };
		jvmManager->OnMIDIEvent(m3->deviceId, raw, 3);
	}

	// --- SysEx ≤ 1 024 ------------------------------------------
	std::unique_ptr<MidiSyx1k> m1k;
	while ((m1k = this->incomingMidiQueue1k.pop()) != nullptr)
		jvmManager->OnMIDIEvent(m1k->deviceId, m1k->data, m1k->size);

	// --- SysEx ≤ 65 536 -----------------------------------------
	std::unique_ptr <MidiSyx64k> m64k;
	while ((m64k = this->incomingMidiQueue64k.pop()) != nullptr)
		jvmManager->OnMIDIEvent(m64k->deviceId, m64k->data, m64k->size);
}


void DrivenByMossSurface::SendMIDIEventsToOutputs()
{
	if (this->isShutdown)
		return;

	// Satisfy the C API
	DISABLE_WARNING_ARRAY_POINTER_DECAY

	// ---------- 3‑byte ----------
	std::unique_ptr<Midi3> m3;
	while ((m3 = this->outgoingMidiQueue3.pop()) != nullptr)
		HandleShortMidi(m3->deviceId, m3->status, m3->data1, m3->data2);

	// ---------- ≤ 1 024 ----------
	std::unique_ptr<MidiSyx1k> m1k;
	while ((m1k = this->outgoingMidiQueue1k.pop()) != nullptr)
		HandleSysex(m1k->deviceId, m1k->data, m1k->size);

	// ---------- ≤ 65 536 ----------
	std::unique_ptr<MidiSyx64k> m64;
	while ((m64 = this->outgoingMidiQueue64k.pop()) != nullptr)
		HandleSysex(m64->deviceId, m64->data, m64->size);
}


void DrivenByMossSurface::HandleShortMidi(uint32_t deviceId, uint8_t status, uint8_t data1, uint8_t data2)
{
	midi_Output* midiout = GetMidiOutput(deviceId);
	if (midiout == nullptr)
		return;

	MIDI_event_t event;
	event.frame_offset = 0;
	event.size = 3;
	event.midi_message[0] = status;
	event.midi_message[1] = data1;
	event.midi_message[2] = data2;
	midiout->SendMsg(&event, -1);
}


void DrivenByMossSurface::HandleSysex(uint32_t deviceId, const uint8_t* data, uint32_t size)
{
	midi_Output* midiout = GetMidiOutput(deviceId);
	if (midiout == nullptr)
		return;

	// Dynamically allocate memory for the MIDI_event_t and the data
	// Subtract 4 because MIDI_event_t already includes the first 4 bytes
	size_t eventSize = sizeof(MIDI_event_t);
	if (size > 4)
		eventSize += (size - 4);

	std::vector<std::uint8_t> buffer(eventSize);
	// Cannot avoid this
	DISABLE_WARNING_REINTERPRET_CAST
	auto* evt = reinterpret_cast<MIDI_event_t*>(buffer.data());

	evt->frame_offset = 0;
	evt->size = size;
	// Both have to be like this to fulfil the C API
	DISABLE_WARNING_ARRAY_POINTER_DECAY
	DISABLE_WARNING_BUFFER_OVERFLOW
	std::memcpy(evt->midi_message, data, size);

	midiout->SendMsg(evt, -1);
}