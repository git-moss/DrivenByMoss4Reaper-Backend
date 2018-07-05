// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "reaper_plugin_functions.h"
#include "Model.h"


/**
 * Collects all data from the Reaper model.
 */
class DataCollector
{
public:
	DataCollector(Model *model);
	~DataCollector();

	std::string DataCollector::CollectData(const bool &dump);

private:
	Model *model;

	const std::regex trackLockPattern{ "LOCK (\\d+)" };
	const std::regex presetPattern{ "Name=(.*)\n" };

	std::string iniPath;

	// Project values
	std::string projectName;
	int projectEngine;

	// Clip values
	double clipMusicalStart;
	double clipMusicalEnd;
	double globalMusicalLoopStart;
	double globalMusicalLoopEnd;
	int globalTimesig;
	int globalDenomOut;
	double playPosition;
	std::string strPlayPosition;
	std::string strBeatPosition;
	int prerollMeasures;
	int prerollClick;

	// Transport values
	int play;
	int record;
	int repeat;
	int metronome;
	double tempo;

	// Track values
	std::vector<int> trackExists;
	std::vector<int> trackNumber;
	std::vector<std::string> trackName;
	std::vector<std::string> trackType;
	std::vector<int> trackSelected;
	std::vector<int> trackMute;
	std::vector<int> trackSolo;
	std::vector<int> trackRecArmed;
	std::vector<int> trackActive;
	std::vector<int> trackMonitor;
	std::vector<int> trackAutoMonitor;
	std::vector<std::string> trackColor;
	std::vector<std::string> trackVolumeStr;
	std::vector<std::string> trackPanStr;
	std::vector<double> trackVULeft;
	std::vector<double> trackVURight;
	std::vector<int> trackAutoMode;
	std::vector<std::vector<std::string>> trackSendName;
	std::vector<std::vector<std::string>> trackSendVolumeStr;
	std::vector<int> trackRepeatActive;
	std::vector<int> trackRepeatNoteLength;

	// Master track values
	int masterSelected;
	int masterMute;
	int masterSolo;
	std::string masterVolumeStr;
	std::string masterPanStr;
	double masterVULeft;
	double masterVURight;

	// Device values
	int deviceExists = -1;
	int devicePosition = -1;
	int deviceWindow = -1;
	int deviceExpanded = -1;
	std::string deviceName;
	int deviceBypass;
	std::vector<std::string> deviceSiblings;
	std::vector<std::string> deviceParamName;
	std::vector<double> deviceParamValue;
	std::vector<std::string> deviceParamValueStr;

	// Browser values
	std::string devicePresetName;
	int devicePresetIndex;
	std::vector<std::string> devicePresetsStr;

	// Groove values
	int grooveStrength;
	int grooveVelstrength;
	int grooveTarget;
	int grooveTolerance;
	int quantizeStrength;


	void CollectProjectData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectTransportData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectDeviceData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectTrackData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectMasterTrackData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectClipData(std::stringstream &ss, ReaProject *project, const bool &dump);
	void CollectBrowserData(std::stringstream &ss, ReaProject *project, const bool &dump);

	void AdjustTrackBank(ReaProject *project);
	int GetTrackLockState(MediaTrack *track);
	std::string FormatColor(int red, int green, int blue);
	std::string FormatDB(double value);
	std::string FormatPan(double value);
	void LoadDevicePresetFile(std::stringstream &ss, MediaTrack *track, int fx, const bool &dump);

	const char *CollectStringValue(std::stringstream &ss, const char *command, std::string currentValue, const char *newValue, const bool &dump) const;
	int CollectIntValue(std::stringstream &ss, const char *command, int currentValue, const int newValue, const bool &dump) const;
	double CollectDoubleValue(std::stringstream &ss, const char *command, double currentValue, const double newValue, const bool &dump) const;

	void CollectStringArrayValue(std::stringstream &ss, const char *command, int index, std::vector<std::string> &currentValues, const char *newValue, const bool &dump) const;
	void CollectIntArrayValue(std::stringstream &ss, const char *command, int index, std::vector<int> &currentValues, int newValue, const bool &dump) const;
	void CollectDoubleArrayValue(std::stringstream &ss, const char *command, int index, std::vector<double> &currentValues, double newValue, const bool &dump) const;

	ReaProject * GetProject()
	{
		// Current project
		const int projectID = -1;
		return EnumProjects(projectID, nullptr, 0);
	};
};
