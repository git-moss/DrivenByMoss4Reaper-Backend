// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_TRANSPORTPROCESSOR_H_
#define _DBM_TRANSPORTPROCESSOR_H_

#include <string>

#include "OscProcessor.h"
#include "ReaperUtils.h"
#include "StringUtils.h"


class PlayProcessor : public OscProcessor
{
public:
	PlayProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};
	PlayProcessor(const PlayProcessor&) = delete;
	PlayProcessor& operator=(const PlayProcessor&) = delete;
	PlayProcessor(PlayProcessor&&) = delete;
	PlayProcessor& operator=(PlayProcessor&&) = delete;
	~PlayProcessor() {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		const int playState = GetPlayStateEx(ReaperUtils::GetProject());
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
	StopProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};
	StopProcessor(const StopProcessor&) = delete;
	StopProcessor& operator=(const StopProcessor&) = delete;
	StopProcessor(StopProcessor&&) = delete;
	StopProcessor& operator=(StopProcessor&&) = delete;
	~StopProcessor() {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		CSurf_OnStop();
	};
};

class RecordProcessor : public OscProcessor
{
public:
	RecordProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		CSurf_OnRecord();
	};
};

class RepeatProcessor : public OscProcessor
{
public:
	RepeatProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Main_OnCommandEx(TRANSPORT_REPEAT, 0, ReaperUtils::GetProject());
	};
};

class TimeProcessor : public OscProcessor
{
public:
	TimeProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		this->Process(path, static_cast<double> (value));
	};

	void Process(std::deque<std::string>& path, double value) noexcept override
	{
		ReaProject* const project = ReaperUtils::GetProject();
		const double end = GetProjectLength(project);
		SetEditCurPos2(project, value < end ? value : end, true, true);
	};
};

class TempoProcessor : public OscProcessor
{
public:
	TempoProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		if (path.empty())
			return;
		ReaProject* project = ReaperUtils::GetProject();
		const char* direction = safeGet(path, 0);
		if (strcmp(direction, "+") == 0)
			Main_OnCommandEx(TEMPO_INC_SLOW, 0, project);
		else if (strcmp(direction, "++") == 0)
			Main_OnCommandEx(TEMPO_INC, 0, project);
		else if (strcmp(direction, "-") == 0)
			Main_OnCommandEx(TEMPO_DEC_SLOW, 0, project);
		else if (strcmp(direction, "--") == 0)
			Main_OnCommandEx(TEMPO_DEC, 0, project);
	};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		CSurf_OnTempoChange(value);
	}

	void Process(std::deque<std::string>& path, double value) noexcept override
	{
		CSurf_OnTempoChange(value);
	}
};

class QuantizeProcessor : public OscProcessor
{
public:
	QuantizeProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, double value) noexcept override
	{
		if (!path.empty())
			return;

		// Amount value currently not possible to use

		HWND activeMidiEditor = MIDIEditor_GetActive();
		const bool isNotOpen = activeMidiEditor == nullptr;

		// MAIN section action 40153: "open selected item in MIDI editor"
		Main_OnCommand(40153, 0);
		activeMidiEditor = MIDIEditor_GetActive();
		// Select all notes
		MIDIEditor_OnCommand(activeMidiEditor, 40003);
		// Quantize all notes
		MIDIEditor_OnCommand(activeMidiEditor, 40728);
		// Close Window (only if it was not open before)
		if (isNotOpen)
			MIDIEditor_OnCommand(activeMidiEditor, 2);
	};
};

class MetronomeVolumeProcessor : public OscProcessor
{
public:
	MetronomeVolumeProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		if (path.empty())
			return;
		const char* direction = safeGet(path, 0);
		const int actionID = NamedCommandLookup(strcmp(direction, "+") == 0 ? "_S&M_METRO_VOL_UP" : "_S&M_METRO_VOL_DOWN");
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, ReaperUtils::GetProject());
	};
};

class UndoProcessor : public OscProcessor
{
public:
	UndoProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Undo_DoUndo2(ReaperUtils::GetProject());
	};
};

class RedoProcessor : public OscProcessor
{
public:
	RedoProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Undo_DoRedo2(ReaperUtils::GetProject());
	};
};

class CursorProcessor : public OscProcessor
{
public:
	CursorProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		CSurf_OnArrow(value, 0);
	};
};

class ProjectProcessor : public OscProcessor
{
public:
	ProjectProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		if (path.empty() || std::strcmp(safeGet(path, 0), "engine") != 0)
			return;
		if (value > 0)
			Audio_Init();
		else
			Audio_Quit();
	};
};

class RefreshProcessor : public OscProcessor
{
public:
	RefreshProcessor(Model& aModel) noexcept : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		try
		{
			this->model.SetDump();
		}
		catch (...)
		{
			// Ignore
		}
	};
};

#endif /* _DBM_TRANSPORTPROCESSOR_H_ */