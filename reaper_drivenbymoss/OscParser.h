// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <iterator>

#include "Model.h"
#include "OscProcessor.h"
#include "JvmManager.h"
#include "TransportProcessor.h"
#include "MastertrackProcessor.h"
#include "TrackProcessor.h"
#include "DeviceProcessor.h"
#include "ClipProcessor.h"
#include "MarkerProcessor.h"
#include "SceneProcessor.h"
#include "NoteRepeatProcessor.h"


/**
 * Parse and execute the received OSC style commands.
 */
class OscParser
{
public:
	OscParser(Model &model);
	virtual ~OscParser() {};

	virtual void Process(const std::string processor, const std::string path) const;
	virtual void Process(const std::string processor, const std::string path, const std::string value) const;
	virtual void Process(const std::string processor, const std::string path, const int value) const;
	virtual void Process(const std::string processor, const std::string path, const double value) const;

private:
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
	ClipProcessor 					 clipProcessor;
	MarkerProcessor 				 markerProcessor;
	RefreshProcessor 				 refreshProcessor;
	SceneProcessor 					 sceneProcessor;
	IniFileProcessor                 iniFileProcessor;

	std::map<std::string, OscProcessor *> processors;
	Model &theModel;

	std::deque<std::string> Split(const std::string &path) const;
	void LogError(const std::string command, const std::out_of_range &oor) const;
};
