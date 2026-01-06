// Copyright (c) 2018-2026 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_DRIVENBYMOSSSURFACE_H_
#define _DBM_DRIVENBYMOSSSURFACE_H_

#include <mutex>
#include <gsl/span>

#include "FunctionExecutor.h"
#include "OscParser.h"
#include "JvmManager.h"
#include "DataCollector.h"
#include "ReaderWriterQueue.h"
#include "MidiMessages.h"


/**
 * Surface implementation.
 */
class DrivenByMossSurface : public IReaperControlSurface
{
public:
	bool isInfrastructureUp{ false };
	bool isShutdown{ false };
	std::unique_ptr<JvmManager>& jvmManager;

	// Queues to transfer incoming MIDI from the audio thread to Java
	ReaderWriterQueue<Midi3, 1024>      incomingMidiQueue3;
	ReaderWriterQueue<MidiSyx1k, 128>  incomingMidiQueue1k;
	ReaderWriterQueue<MidiSyx64k, 16> incomingMidiQueue64k;

	// Queues to transfer MIDI from Java to an output port in the audio thread
	ReaderWriterQueue<Midi3, 1024>      outgoingMidiQueue3;
	ReaderWriterQueue<MidiSyx1k, 128>  outgoingMidiQueue1k;
	ReaderWriterQueue<MidiSyx64k, 16> outgoingMidiQueue64k;


	DrivenByMossSurface(std::unique_ptr<JvmManager>& aJvmManager, midi_Output* (*aGetMidiOutput)(int idx));
	DrivenByMossSurface(const DrivenByMossSurface&) = delete;
	DrivenByMossSurface& operator=(const DrivenByMossSurface&) = delete;
	DrivenByMossSurface(DrivenByMossSurface&&) = delete;
	DrivenByMossSurface& operator=(DrivenByMossSurface&&) = delete;
	~DrivenByMossSurface();

	void Shutdown();

#ifdef _WIN32
	// Prevents warning that the class is not 64 bit aligned
	void* operator new(size_t i) noexcept
	{
		return _mm_malloc(i, 64);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}
#endif

	OscParser& GetOscParser() noexcept
	{
		return this->oscParser;
	}

	DataCollector& GetDataCollector() noexcept
	{
		return this->dataCollector;
	}

	const char* GetTypeString() noexcept override;
	const char* GetDescString() noexcept override;
	const char* GetConfigString() noexcept override;

	void Run() override;

	void SetTrackListChange() noexcept override;
	void SetSurfaceVolume(MediaTrack* trackid, double volume) noexcept override;
	void SetSurfacePan(MediaTrack* trackid, double pan) noexcept override;
	void SetSurfaceMute(MediaTrack* trackid, bool mute) noexcept override;
	void SetSurfaceSelected(MediaTrack* trackid, bool selected) noexcept override;
	void SetSurfaceSolo(MediaTrack* trackid, bool solo) noexcept override;
	void SetSurfaceRecArm(MediaTrack* trackid, bool recarm) noexcept override;
	void SetPlayState(bool play, bool pause, bool rec) noexcept override;
	void SetRepeatState(bool rep) noexcept override;
	void SetTrackTitle(MediaTrack* trackid, const char* title) noexcept override;
	bool GetTouchState(MediaTrack* trackid, int isPan) override;
	void SetAutoMode(int mode) noexcept override;
	void ResetCachedVolPanStates() noexcept override;
	void OnTrackSelection(MediaTrack* trackid) noexcept override;

	// These helpers touch no locks, no heap, and cost a single memcpy per message.
	inline void EnqueueMidi3(uint32_t dev, uint8_t status, uint8_t d1 = 0, uint8_t d2 = 0)
	{
		std::unique_ptr<Midi3> m = std::make_unique<Midi3>();
		m->deviceId = dev;
		m->status = status;
		m->data1 = d1;
		m->data2 = d2;
		this->incomingMidiQueue3.push(std::move(m));
	}

	inline void EnqueueSysex1k(uint32_t dev, const uint8_t* buf, uint32_t len)
	{
		if (len == 0 || len > kSyx1k_Max)
			return;
		
		std::unique_ptr<MidiSyx1k> m = std::make_unique<MidiSyx1k>();
		m->deviceId = dev;
		m->size = len;

		const gsl::span<uint8_t> destinationSpan(m->data);
		const gsl::span<const uint8_t> sourceSpan(buf, len);
		std::copy(sourceSpan.begin(), sourceSpan.end(), destinationSpan.begin());
		incomingMidiQueue1k.push(std::move(m));
	}

	inline void EnqueueSysex64k(uint32_t dev, const uint8_t* buf, uint32_t len)
	{
		if (len == 0 || len > kSyx64k_Max)
			return;
		
		std::unique_ptr<MidiSyx64k> m = std::make_unique<MidiSyx64k>();
		m->deviceId = dev;
		m->size = len;

		const gsl::span<uint8_t> destinationSpan(m->data);
		const gsl::span<const uint8_t> sourceSpan(buf, len);
		std::copy(sourceSpan.begin(), sourceSpan.end(), destinationSpan.begin());
		incomingMidiQueue64k.push(std::move(m));
	}

	void SendMIDIEventsToOutputs();

private:
	midi_Output* (*GetMidiOutput)(int idx);
	FunctionExecutor functionExecutor;
	Model model;
	OscParser oscParser{ model };
	DataCollector dataCollector{ model };
	bool updateModel{ false };
	std::mutex startInfrastructureMutex;

	std::string CollectData(bool dump)
	{
		return this->dataCollector.CollectData(dump, this->oscParser.GetActionProcessor());
	};

	void SendMIDIEventsToJava();

	void HandleShortMidi(uint32_t deviceId, uint8_t status, uint8_t data1, uint8_t data2);
	void HandleSysex(uint32_t deviceId, const uint8_t* data, uint32_t size);
};


#endif /* _DBM_DRIVENBYMOSSSURFACE_H_ */
