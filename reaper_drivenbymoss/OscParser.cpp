// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "OscParser.h"
#include "TransportProcessor.h"
#include "MastertrackProcessor.h"
#include "TrackProcessor.h"
#include "DeviceProcessor.h"
#include "ClipProcessor.h"
#include "MidiProcessor.h"
#include "GrooveProcessor.h"
#include "NotifyProcessor.h"


/**
 * Constructor.
 */
OscParser::OscParser(Model *model)
{
	this->processors["play"] = new PlayProcessor();
	this->processors["stop"] = new StopProcessor();
	this->processors["record"] = new RecordProcessor();
	this->processors["repeat"] = new RepeatProcessor();
	this->processors["time"] = new TimeProcessor();
	this->processors["tempo"] = new TempoProcessor();
	this->processors["action"] = new ActionProcessor();
	this->processors["action_ex"] = new ActionExProcessor();
	this->processors["quantize"] = new QuantizeProcessor();
	this->processors["metro_vol"] = new MetronomeVolumeProcessor();
	this->processors["preroll"] = new PrerollProcessor();
	this->processors["undo"] = new UndoProcessor();
	this->processors["redo"] = new RedoProcessor();
	this->processors["cursor"] = new CursorProcessor();
	this->processors["project"] = new ProjectProcessor();
	this->processors["master"] = new MastertrackProcessor(model);
	this->processors["track"] = new TrackProcessor(model);
	this->processors["device"] = new DeviceProcessor(model);
	this->processors["clip"] = new ClipProcessor(model);
	this->processors["vkb_midi"] = new MidiProcessor();
	this->processors["groove"] = new GrooveProcessor();
	this->processors["notify"] = new NotifyProcessor();
}


/**
 * Destructor.
 */
OscParser::~OscParser()
{
	// Iterate over processors and delete them
	for (auto const &entry : this->processors)
		delete entry.second;
}


/**
 * Process an OSC style command.
 *
 * @param command The command
 */
void OscParser::Process(const std::string &command) const
{
	std::deque<std::string> elements = split(command);
	if (elements.empty())
		return;
	std::string cmd = elements.front();
	OscProcessor *processor = GetProcessor(cmd);
	if (processor == nullptr)
		return;
	elements.pop_front();
	processor->Process(cmd, elements);
}

/**
 * Process an OSC style command.
 *
 * @param command The command
 * @aram value The value
 */
void OscParser::Process(const std::string &command, const char *value) const
{
	std::deque<std::string> elements = split(command);
	if (elements.empty())
		return;
	std::string cmd = elements.front();
	OscProcessor *processor = GetProcessor(cmd);
	if (processor == nullptr)
		return;
	elements.pop_front();
	processor->Process(cmd, elements, value);
}


/**
* Process an OSC style command.
*
* @param command The command
* @param value The value
*/
void OscParser::Process(const std::string &command, const int &value) const
{
	std::deque<std::string> elements = split(command);
	if (elements.empty())
		return;
	std::string cmd = elements.front();
	OscProcessor *processor = GetProcessor(cmd);
	if (processor == nullptr)
		return;
	elements.pop_front();
	processor->Process(cmd, elements, value);
}


/**
* Process an OSC style command.
*
* @param command The command
* @param value The value
*/
void OscParser::Process(const std::string &command, const double &value) const
{
	std::deque<std::string> elements = split(command);
	if (elements.empty())
		return;
	std::string cmd = elements.front();
	OscProcessor *processor = GetProcessor(cmd);
	if (processor == nullptr)
		return;
	elements.pop_front();
	processor->Process(cmd, elements, value);
}


/**
 * Get a processor for the command.
 *
 * @param command The command
 * @return The processor or nullptr if not found
 */
OscProcessor *OscParser::GetProcessor(const std::string &command) const
{
	try
	{
		return this->processors.at(command);
	}
	catch (const std::out_of_range &oor)
	{
		(void)oor; // Ignore not used
		std::ostringstream message;
		message << "No function " << command << " registered!";
		ReaScriptError(message.str().c_str());
		return nullptr;
	}
}


/**
 * Splits the given path (limiter is "/").
 *
 * @param path The path to split
 * @return The parts in a queue
 */
std::deque<std::string> OscParser::split(const std::string &path) const
{
	std::deque<std::string> elems;
	std::stringstream ss(path);
	std::string item;
	std::back_insert_iterator<std::deque<std::string>> result = back_inserter(elems);
	while (getline(ss, item, '/'))
		*(result++) = item;

	// No command at all?
	if (elems.size() < 2)
		elems.clear();
	else
		// First entry is empty
		elems.pop_front();
	return elems;
}
