// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "Model.h"


/**
 * Collects all data from the Reaper model.
 */
class DataCollector
{
public:
	DataCollector(Model &model);
	virtual ~DataCollector();

	std::string CollectData(const bool &dump);

private:
	Model &model;
	int projectState{ -1 };

	const static int BUFFER_SIZE{ 65535 };
	std::unique_ptr<char []> trackStateChunk;


	const std::regex trackLockPattern{ "LOCK (\\d+)" };
	const std::regex presetPattern{ "Name=(.*)" };

	// Project values
	std::string projectName{};
	int projectEngine{};

	// Clip values
	std::string clipColor{};
	double clipMusicalStart{};
	double clipMusicalEnd{};
	double clipMusicalPlayPosition{};
	int clipLoopIsEnabled{};
	// Note hash
	std::string noteHash{};
	std::string notesStr{};
	std::string formattedClips{};

	// Transport values
	int globalTimesig{};
	int globalDenomOut{};
	double playPosition{};
	std::string strPlayPosition{};
	std::string strBeatPosition{};
	int prerollMeasures{};
	int prerollClick{};
	int play{};
	int record{};
	int repeat{};
	int metronome{};
	double tempo{};

	// Master track values
	int masterSelected{};
	int masterMute{};
	int masterSolo{};
	std::string masterVolumeStr{};
	std::string masterPanStr{};
	double masterVULeft{};
	double masterVURight{};

	// Device values
	int deviceExists{ -1 };
	int devicePosition{ -1 };
	int deviceWindow{ -1 };
	int deviceExpanded{ -1 };
	std::string deviceName{};
	int deviceBypass{};
	std::vector<std::string> deviceSiblings;

	// Browser values
	std::string devicePresetName{};
	int devicePresetIndex{};
	std::vector<std::string> devicePresetsStr;

	// Groove values
	int grooveStrength{};
	int grooveVelstrength{};
	int grooveTarget{};
	int grooveTolerance{};
	int quantizeStrength{};


	void CollectProjectData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectTransportData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectDeviceData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectTrackData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectMasterTrackData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectBrowserData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectMarkerData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectClipData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectSessionData(std::stringstream &ss, ReaProject *project, const bool &dump);

	std::string CollectClipNotes(ReaProject *project, MediaItem *item);
	void LoadDevicePresetFile(std::stringstream &ss, MediaTrack *track, int fx, const bool &dump);
};
