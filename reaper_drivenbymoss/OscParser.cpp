// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "OscParser.h"
#include "ReaDebug.h"


/**
* Constructor.
*
* @param model The model to share data
*/
OscParser::OscParser(Model &model) :
	playProcessor(model),
	stopProcessor(model),
	recordProcessor(model),
	repeatProcessor(model),
	timeProcessor(model),
	tempoProcessor(model),
	actionProcessor(model),
	quantizeProcessor(model),
	metronomeVolumeProcessor(model),
	undoProcessor(model),
	redoProcessor(model),
	cursorProcessor(model),
	projectProcessor(model),
	mastertrackProcessor(model),
	trackProcessor(model),
	deviceProcessor(model),
	clipProcessor(model),
	markerProcessor(model),
	refreshProcessor(model),
	sceneProcessor(model),
	theModel(model)
{
	this->processors["play"] = &playProcessor;
	this->processors["stop"] = &stopProcessor;
	this->processors["record"] = &recordProcessor;
	this->processors["repeat"] = &repeatProcessor;
	this->processors["time"] = &timeProcessor;
	this->processors["tempo"] = &tempoProcessor;
	this->processors["action"] = &actionProcessor;
	this->processors["quantize"] = &quantizeProcessor;
	this->processors["metro_vol"] = &metronomeVolumeProcessor;
	this->processors["undo"] = &undoProcessor;
	this->processors["redo"] = &redoProcessor;
	this->processors["cursor"] = &cursorProcessor;
	this->processors["project"] = &projectProcessor;
	this->processors["master"] = &mastertrackProcessor;
	this->processors["track"] = &trackProcessor;
	this->processors["device"] = &deviceProcessor;
	this->processors["clip"] = &clipProcessor;
	this->processors["marker"] = &markerProcessor;
	this->processors["refresh"] = &refreshProcessor;
	this->processors["scene"] = &sceneProcessor;
}


/**
 * Process an OSC style command.
 *
 * @param processor The processor
 * @param command   The command
 */
void OscParser::Process(const std::string processor, const std::string command) const
{
	this->theModel.AddFunction([this, processor, command]()
	{
		std::deque<std::string> elements = this->Split(command);
		try
		{
			this->processors.at(processor)->Process(elements);
		}
		catch (const std::out_of_range &oor)
		{
			LogError(command, oor);
		}
	});
}


/**
 * Process an OSC style command.
 *
 * @param processor The processor
 * @param command   The command
 * @param value     The value
 */
void OscParser::Process(const std::string processor, const std::string command, const std::string value) const
{
	this->theModel.AddFunction([this, processor, command, value]()
	{
		std::deque<std::string> elements = this->Split(command);
		try
		{
			this->processors.at(processor)->Process(elements, value);
		}
		catch (const std::out_of_range &oor)
		{
			LogError(command, oor);
		}
	});
}


/**
 * Process an OSC style command.
 *
 * @param processor The processor
 * @param command   The command
 * @param value     The value
 */
void OscParser::Process(const std::string processor, const std::string command, const int value) const
{
	this->theModel.AddFunction([this, processor, command, value]()
	{
		std::deque<std::string> elements = this->Split(command);
		try
		{
			this->processors.at(processor)->Process(elements, value);
		}
		catch (const std::out_of_range &oor)
		{
			LogError(command, oor);
		}
	});
}


/**
 * Process an OSC style command.
 *
 * @param processor The processor
 * @param command   The command
 * @param value     The value
 */
void OscParser::Process(const std::string processor, const std::string command, const double value) const
{
	this->theModel.AddFunction([this, processor, command, value]()
	{
		std::deque<std::string> elements = this->Split(command);
		try
		{
			this->processors.at(processor)->Process(elements, value);
		}
		catch (const std::out_of_range &oor)
		{
			LogError(command, oor);
		}
	});
}


/**
 * Get a processor for the command.
 *
 * @param command The command
 * @param oor     The out of range exception
 */
void OscParser::LogError(const std::string command, const std::out_of_range &oor) const
{
	(void)oor; // Ignore not used
	ReaDebug() << "No function " << command << " registered!";
}


/**
 * Splits the given path (limiter is "/").
 *
 * @param path The path to split
 * @return The parts in a queue
 */
std::deque<std::string> OscParser::Split(const std::string &path) const
{
	std::deque<std::string> elems;
	std::stringstream ss(path);
	std::string item;
	std::back_insert_iterator<std::deque<std::string>> result = back_inserter(elems);
	while (getline(ss, item, '/'))
		*(result++) = item;
	return elems;
}
