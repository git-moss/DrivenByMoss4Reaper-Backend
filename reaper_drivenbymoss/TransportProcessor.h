// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


class PlayProcessor : public OscProcessor
{
public:
	PlayProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		const int playState = GetPlayStateEx(this->model.GetProject());
		if (playState & 1)
			CSurf_OnPlay();
		else
		{
			CSurf_OnPause();
			if ((playState & 2) == 0)
				CSurf_OnPlay();
		}
	};
};

class StopProcessor : public OscProcessor
{
public:
	StopProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		CSurf_OnStop();
	};
};

class RecordProcessor : public OscProcessor
{
public:
	RecordProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		CSurf_OnRecord();
	};
};

class RepeatProcessor : public OscProcessor
{
public:
	RepeatProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		Main_OnCommandEx(1068, 0, this->model.GetProject());
	};
};

class TimeProcessor : public OscProcessor
{
public:
	TimeProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		this->Process(command, path, static_cast<double> (value));
	};

	void Process(std::string command, std::deque<std::string> &path, double value) override
	{
		ReaProject * const project = this->model.GetProject();
		const double end = GetProjectLength(project);
		SetEditCurPos2(project, value < end ? value : end, true, true);
	};
};

class TempoProcessor : public OscProcessor
{
public:
	TempoProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		ReaProject *project = this->model.GetProject();
		const char *direction = path.at(0).c_str();
		if (strcmp(direction, "+") == 0)
			Main_OnCommandEx(41137, 0, project);
		else if (strcmp(direction, "++") == 0)
			Main_OnCommandEx(41129, 0, project);
		else if (strcmp(direction, "-") == 0)
			Main_OnCommandEx(41138, 0, project);
		else if (strcmp(direction, "--") == 0)
			Main_OnCommandEx(41130, 0, project);
	};

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		CSurf_OnTempoChange(value);
	}

	void Process(std::string command, std::deque<std::string> &path, double value) override
	{
		CSurf_OnTempoChange(value);
	}
};

class ActionProcessor : public OscProcessor
{
public:
	ActionProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		Main_OnCommandEx(value, 0, this->model.GetProject());
	};
};

class ActionExProcessor : public OscProcessor
{
public:
	ActionExProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path, const std::string &value) override
	{
		const int actionID = NamedCommandLookup(value.c_str());
		if (actionID <= 0)
			return;
		Main_OnCommandEx(actionID, 0, this->model.GetProject());
	};
};

class QuantizeProcessor : public OscProcessor
{
public:
	QuantizeProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		// MAIN section action 40153: "open selected item in MIDI editor"
		Main_OnCommand(40153, 0);
		HWND active_MIDI_editor = MIDIEditor_GetActive();
		// Select all notes
		MIDIEditor_OnCommand(active_MIDI_editor, 40003);
		// Quantize all notes
		MIDIEditor_OnCommand(active_MIDI_editor, 40728);
		// Close Window
		MIDIEditor_OnCommand(active_MIDI_editor, 2);
	};
};

class MetronomeVolumeProcessor : public OscProcessor
{
public:
	MetronomeVolumeProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		if (path.empty())
			return;
		const char *direction = path.at(0).c_str();
		const int actionID = NamedCommandLookup(strcmp(direction, "+") == 0 ? "_S&M_METRO_VOL_UP" : "_S&M_METRO_VOL_DOWN");
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, this->model.GetProject());
	};
};

class UndoProcessor : public OscProcessor
{
public:
	UndoProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		Undo_DoUndo2(this->model.GetProject());
	};
};

class RedoProcessor : public OscProcessor
{
public:
	RedoProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path) override
	{
		Undo_DoRedo2(this->model.GetProject());
	};
};

class CursorProcessor : public OscProcessor
{
public:
	CursorProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		CSurf_OnArrow(value, 0);
	};
};

class ProjectProcessor : public OscProcessor
{
public:
	ProjectProcessor(Model &aModel) : OscProcessor(aModel) {};

	void Process(std::string command, std::deque<std::string> &path, int value) override
	{
		if (path.empty() || std::strcmp(path.at(0).c_str(), "engine") != 0)
			return;
		if (value > 0)
			Audio_Init();
		else
			Audio_Quit();
	};
};
