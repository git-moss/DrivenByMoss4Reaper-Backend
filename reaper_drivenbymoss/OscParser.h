// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_OSCPARSER_H_
#define _DBM_OSCPARSER_H_

#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <iterator>

#include "Model.h"
#include "OscProcessor.h"
#include "JvmManager.h"
#include "ActionProcessor.h"
#include "TransportProcessor.h"
#include "MastertrackProcessor.h"
#include "TrackProcessor.h"
#include "DeviceProcessor.h"
#include "EqDeviceProcessor.h"
#include "ClipProcessor.h"
#include "MarkerProcessor.h"
#include "ProjectProcessor.h"
#include "SceneProcessor.h"
#include "NoteRepeatProcessor.h"
#include "GrooveProcessor.h"
#include "IniFileProcessor.h"


/**
 * Parse and execute the received OSC style commands.
 */
class OscParser
{
public:
	OscParser(Model& model);
	OscParser(const OscParser&) = delete;
	OscParser& operator=(const OscParser&) = delete;
	OscParser(OscParser&&) = delete;
	OscParser& operator=(OscParser&&) = delete;
	virtual ~OscParser() {};

	virtual void Process(const std::string processor, const std::string path) const;
	virtual void Process(const std::string processor, const std::string path, const std::string value) const;
	virtual void Process(const std::string processor, const std::string command, std::vector<std::string>& values) const;
	virtual void Process(const std::string processor, const std::string path, const int value) const;
	virtual void Process(const std::string processor, const std::string path, const double value) const;

	ActionProcessor& GetActionProcessor() noexcept
	{
		return this->actionProcessor;
	};


private:
	AutomationProcessor 			 automationProcessor;
	PlayProcessor 					 playProcessor;
	StopProcessor 					 stopProcessor;
	RecordProcessor 				 recordProcessor;
	RepeatProcessor 				 repeatProcessor;
	TimeProcessor 					 timeProcessor;
	TempoProcessor 					 tempoProcessor;
	ActionProcessor 				 actionProcessor;
	QuantizeProcessor 				 quantizeProcessor;
	MetronomeVolumeProcessor 		 metronomeVolumeProcessor;
	UndoProcessor 					 undoProcessor;
	RedoProcessor 					 redoProcessor;
	CursorProcessor 				 cursorProcessor;
	ProjectProcessor 				 projectProcessor;
	MastertrackProcessor 			 mastertrackProcessor;
	TrackProcessor 					 trackProcessor;
	NoteRepeatProcessor              noteRepeatProcessor;
	DeviceProcessor 				 deviceProcessor;
	EqDeviceProcessor 				 eqDeviceProcessor;
	ClipProcessor 					 clipProcessor;
	MarkerProcessor 				 markerProcessor;
	RefreshProcessor 				 refreshProcessor;
	SceneProcessor 					 sceneProcessor;
	GrooveProcessor 				 grooveProcessor;
	IniFileProcessor                 iniFileProcessor;

	std::map<std::string, OscProcessor*> processors;
	Model& theModel;

	std::deque<std::string> Split(const std::string& path) const;
	void LogError(const std::string command, const std::out_of_range& oor) const;
};

#endif /* _DBM_OSCPARSER_H_ */