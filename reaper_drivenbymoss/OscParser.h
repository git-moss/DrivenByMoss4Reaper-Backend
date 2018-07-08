// Written by J�rgen Mo�graber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <sstream>
#include <deque>
#include <map>
#include <iterator>
#include "reaper_plugin_functions.h"
#include "Model.h"
#include "OscProcessor.h"
#include "JvmManager.h"
#include "TransportProcessor.h"
#include "MastertrackProcessor.h"
#include "TrackProcessor.h"
#include "DeviceProcessor.h"
#include "ClipProcessor.h"
#include "MidiProcessor.h"


/**
 * Parse and execute the received OSC style commands.
 */
class OscParser
{
public:
	OscParser(Model &model);
	virtual ~OscParser() {};

	virtual void Process(const std::string &command) const;
	virtual void Process(const std::string &command, const std::string &value) const;
	virtual void Process(const std::string &command, const int &value) const;
	virtual void Process(const std::string &command, const double &value) const;

private:
	PlayProcessor 					 playProcessor;
	StopProcessor 					 stopProcessor;
	RecordProcessor 				 recordProcessor;
	RepeatProcessor 				 repeatProcessor;
	TimeProcessor 					 timeProcessor;
	TempoProcessor 					 tempoProcessor;
	ActionProcessor 				 actionProcessor;
	ActionExProcessor 				 actionExProcessor;
	QuantizeProcessor 				 quantizeProcessor;
	MetronomeVolumeProcessor 		 metronomeVolumeProcessor;
	UndoProcessor 					 undoProcessor;
	RedoProcessor 					 redoProcessor;
	CursorProcessor 				 cursorProcessor;
	ProjectProcessor 				 projectProcessor;
	MastertrackProcessor 			 mastertrackProcessor;
	TrackProcessor 					 trackProcessor;
	DeviceProcessor 				 deviceProcessor;
	ClipProcessor 					 clipProcessor;
	MidiProcessor 					 midiProcessor;

	std::map<std::string, OscProcessor *> processors;

	std::deque<std::string> Split(const std::string &path) const;
	void LogError(const std::string command, const std::out_of_range &oor) const;
};
