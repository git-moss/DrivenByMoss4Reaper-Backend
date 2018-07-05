// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "MidiProcessor.h"


void MidiProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.empty())
		return;
	int channel = std::atoi(path[0].c_str());

	const char *cmd = path[1].c_str();

	if (std::strcmp(cmd, "note") == 0)
	{
		int number = atoi(path[2].c_str());
		StuffMIDIMessage(0, 0x90 + channel, number, value);
	}
	else if (std::strcmp(cmd, "aftertouch") == 0)
	{
		int number = atoi(path[2].c_str());
		StuffMIDIMessage(0, 0xA0 + channel, number, value);
	}
	else if (std::strcmp(cmd, "cc") == 0)
	{
		int number = atoi(path[2].c_str());
		StuffMIDIMessage(0, 0xB0 + channel, number, value);
	}
	else if (std::strcmp(cmd, "pitch") == 0)
	{
		StuffMIDIMessage(0, 0xE0 + channel, value % 128, value / 128);
	}
	else if (std::strcmp(cmd, "program") == 0)
	{
		StuffMIDIMessage(0, 0xC0 + channel, value, 0);
	}
}