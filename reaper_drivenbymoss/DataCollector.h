// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_DATACOLLECTOR_H_
#define _DBM_DATACOLLECTOR_H_

#include <chrono>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "Model.h"
#include "ActionProcessor.h"


/**
 * Collects all data from the Reaper model.
 */
class DataCollector
{
public:
	static const int TIME_LENGTH{ 20 };

	DataCollector(Model& model);
	virtual ~DataCollector();
	DataCollector(const DataCollector&) = delete;
	DataCollector& operator=(const DataCollector&) = delete;
	DataCollector(DataCollector&&) = delete;
	DataCollector& operator=(DataCollector&&) = delete;

	std::string CollectData(const bool& dump, ActionProcessor& actionProcessor);

	void EnableUpdate(std::string processor, bool enable);
	void DelayUpdate(std::string processor);

private:
	const static int DELAY{ 300 };
	static const int SLOW_UPDATE{ 16 };

	std::map<std::string, bool> disableUpdateMap;
	std::map<std::string, long long> delayUpdateMap;
	std::mutex delayMutex;

	int slowCounter{ 0 };


	Model& model;
	int projectState{ -1 };

	const static int BUFFER_SIZE{ 65535 };
	std::unique_ptr<char[]> trackStateChunk;


	const std::regex trackLockPattern{ "LOCK (\\d+)" };
	const std::regex presetHeaderPattern{ "\\[Preset(.*)\\]" };
	const std::regex presetNamePattern{ "Name=(.*)" };

	// Project values
	std::string projectName{};
	int projectEngine{};
	int canUndo{ 1 };
	int canRedo{ 1 };

	// Clip values
	int clipExists{ -1 };
	std::string clipColor{};
	double clipMusicalStart{};
	double clipMusicalEnd{};
	double clipMusicalPlayPosition{};
	int clipLoopIsEnabled{};
	std::string playingNotesStr{};
	// Note hash
	std::string noteHash{};
	std::string notesStr{};
	std::string formattedClips{};

	// Transport values
	int globalTimesig{ -1 };
	int globalDenomOut{ -1 };
	double playPosition{};
	std::string strPlayPosition{};
	std::string strBeatPosition{};
	int play{};
	int record{};
	int repeat{};
	double tempo{};
	int followPlayback{ 0 };
	double loopStart{};
	double loopLength{};
	std::string strLoopStart{};
	std::string strLoopLength{};
	std::string strLoopStartBeat{};
	std::string strLoopLengthBeat{};

	// Click / metronome values
	int metronome{};
	int prerollClick{};
	double metronomeVolume{};
	std::string metronomeVolumeStr;

	// Master track values
	int masterSelected{};
	int masterMute{};
	int masterSolo{};
	std::string masterVolumeStr{};
	std::string masterPanStr{};
	double masterVU{ 0.0 };
	double masterVULeft{ 0.0 };
	double masterVURight{ 0.0 };
	std::string masterColor;
	int masterAutoMode{ 0 };
	Parameter crossfaderParameter;

	// Device values
	int deviceExists{ -1 };
	int devicePosition{ -1 };
	int deviceWindow{ -1 };
	int deviceExpanded{ -1 };
	std::string deviceName{};
	int deviceBypass{};
	std::vector<std::string> deviceSiblings;
	std::vector<int> deviceSiblingsSelection;
	std::vector<int> deviceSiblingsBypass;

	// Instrument device values
	int instrumentExists{ -1 };
	int instrumentPosition{ -1 };
	std::string instrumentName{};
	int instrumentParameterCount{ 0 };
	Parameter instrumentParameter;

	// Equalizer device values
	int eqExists{ -1 };
	std::vector<std::string> eqBandTypes;

	// Browser values
	std::string devicePresetName{};
	int devicePresetIndex{};
	std::vector<std::string> devicePresetsStr;

	// NoteRepeat values
	int repeatActive{ 0 };
	double repeatRate{ 1.0 };
	double repeatNoteLength{ 1.0 };
	int repeatMode{ 0 };
	int repeatVelocity{ 1 };

	// Groove
	int swingActive{ 0 };
	double swingAmount{ 0 };


	bool IsActive(std::string processor);
	bool CheckDelay(std::string processor);

	void CollectProjectData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectTransportData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectDeviceData(std::ostringstream& ss, ReaProject* project, MediaTrack* track, const bool& dump);
	void CollectTrackData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectMasterTrackData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectBrowserData(std::ostringstream& ss, MediaTrack* track, const bool& dump);
	void CollectMarkerData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectClipData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectSessionData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectNoteRepeatData(std::ostringstream& ss, ReaProject* project, const bool& dump);
	void CollectGrooveData(std::ostringstream& ss, ReaProject* project, const bool& dump);

	std::string CollectClipNotes(ReaProject* project, MediaItem* item);
	std::string CollectPlayingNotes(ReaProject* project, MediaTrack* track);

	MediaItem_Take* GetMidiTakeAtPlayPosition(ReaProject* project, MediaTrack* track) const noexcept;
	void LoadDevicePresetFile(std::ostringstream& ss, MediaTrack* track, int fx, const bool& dump);
};

#endif /* _DBM_DATACOLLECTOR_H_ */
