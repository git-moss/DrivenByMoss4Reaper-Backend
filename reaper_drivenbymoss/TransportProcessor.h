// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>

#include "OscProcessor.h"


class PlayProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		const int playState = GetPlayStateEx(this->GetProject());
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
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		CSurf_OnStop();
	};
};

class RecordProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		CSurf_OnRecord();
	};
};

class RepeatProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		Main_OnCommandEx(1068, 0, this->GetProject());
	};
};

class TimeProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		this->Process(command, path, (double) value);
	};

	virtual void Process(std::string command, std::deque<std::string> &path, double value)
	{
		SetEditCurPos2(this->GetProject(), value, true, true);
	};
};

class TempoProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		ReaProject *project = this->GetProject();
		const char *direction = path[0].c_str();
		if (strcmp(direction, "+") == 0)
			Main_OnCommandEx(41137, 0, project);
		else if (strcmp(direction, "++") == 0)
			Main_OnCommandEx(41129, 0, project);
		else if (strcmp(direction, "-") == 0)
			Main_OnCommandEx(41138, 0, project);
		else if (strcmp(direction, "--") == 0)
			Main_OnCommandEx(41130, 0, project);
	};

	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		CSurf_OnTempoChange(value);
	}

	virtual void Process(std::string command, std::deque<std::string> &path, double value)
	{
		CSurf_OnTempoChange(value);
	}
};

class ActionProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		Main_OnCommandEx(value, 0, this->GetProject());
	};
};

class ActionExProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, const char *value)
	{
		int actionID = NamedCommandLookup(value);
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, this->GetProject());
	};
};

class QuantizeProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
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

	virtual void Process(std::string command, std::deque<std::string> &path, const char *value)
	{
		if (path.empty() || strcmp(path[0].c_str(), "strength") != 0 || !APIExists("BR_Win32_WritePrivateProfileString"))
			return;

		const char *inipath = get_ini_file();
		if (inipath)
			BR_Win32_WritePrivateProfileString("midiedit", "quantstrength", value, inipath);
	};
};

class MetronomeVolumeProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		if (path.empty())
			return;
		const char *direction = path[0].c_str();
		int actionID = NamedCommandLookup(strcmp(direction, "+") == 0 ? "_S&M_METRO_VOL_UP" : "_S&M_METRO_VOL_DOWN");
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, this->GetProject());
	};
};

class PrerollProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		// TODO Find alternative, crashes
		if (APIExists("SNM_SetDoubleConfigVar"))
			SNM_SetDoubleConfigVar ("prerollmeas", value);
	};
};

class UndoProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		Undo_DoUndo2(this->GetProject());
	};
};

class RedoProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path)
	{
		Undo_DoRedo2(this->GetProject());
	};
};

class CursorProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		CSurf_OnArrow(value, 0);
	};
};

class ProjectProcessor : public OscProcessor
{
public:
	virtual void Process(std::string command, std::deque<std::string> &path, int value)
	{
		if (path.empty() || strcmp(path[0].c_str(), "engine") != 0)
			return;
		if (value > 0)
			Audio_Init();
		else
			Audio_Quit();
	};
};
