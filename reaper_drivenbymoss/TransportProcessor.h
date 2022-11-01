// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_TRANSPORTPROCESSOR_H_
#define _DBM_TRANSPORTPROCESSOR_H_

#include "WrapperGSL.h"
#include "OscProcessor.h"
#include "ReaperUtils.h"
#include "StringUtils.h"


class PlayProcessor : public OscProcessor
{
public:
	PlayProcessor(Model& aModel) : OscProcessor(aModel) {};
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

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class StopProcessor : public OscProcessor
{
public:
	StopProcessor(Model& aModel) : OscProcessor(aModel) {};
	StopProcessor(const StopProcessor&) = delete;
	StopProcessor& operator=(const StopProcessor&) = delete;
	StopProcessor(StopProcessor&&) = delete;
	StopProcessor& operator=(StopProcessor&&) = delete;
	~StopProcessor() {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		CSurf_OnStop();
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class RecordProcessor : public OscProcessor
{
public:
	RecordProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		CSurf_OnRecord();
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class RepeatProcessor : public OscProcessor
{
public:
	RepeatProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Main_OnCommandEx(TRANSPORT_REPEAT, 0, ReaperUtils::GetProject());
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class TimeProcessor : public OscProcessor
{
public:
	TimeProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		this->Process(path, static_cast<double> (value));
	};

	void Process(std::deque<std::string>& path, double value) noexcept override
	{
		ReaProject* const project = ReaperUtils::GetProject();

		if (path.empty())
		{
			const double end = GetProjectLength(project);
			SetEditCurPos2(project, SnapToGrid(project, value < end ? value : end), true, true);
			return;
		}

		const char* cmd = SafeGet(path, 0);
		if (std::strcmp(cmd, "loop") == 0)
		{
			double startOut;
			double endOut;
			GetSet_LoopTimeRange(false, true, &startOut, &endOut, false);

			const char* loopCmd = SafeGet(path, 1);
			double adjusted = SnapToGrid(project, value);
			if (std::strcmp(loopCmd, "start") == 0)
			{
				const double offset = adjusted - startOut;
				double end = offset + endOut;
				GetSet_LoopTimeRange(true, true, &adjusted, &end, false);
			}
			else
			{
				GetSet_LoopTimeRange(true, true, &startOut, &adjusted, false);
			}
		}
	};

	void Process(std::deque<std::string>& path) noexcept override {};
	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
};

class TempoProcessor : public OscProcessor
{
public:
	TempoProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		if (path.empty())
			return;
		ReaProject* project = ReaperUtils::GetProject();
		const char* direction = SafeGet(path, 0);
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

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
};

class QuantizeProcessor : public OscProcessor
{
public:
	QuantizeProcessor(Model& aModel) : OscProcessor(aModel) {};

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

	void Process(std::deque<std::string>& path) noexcept override {};
	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
};

class MetronomeVolumeProcessor : public OscProcessor
{
public:
	MetronomeVolumeProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		if (path.empty())
			return;
		const char* direction = SafeGet(path, 0);
		const int val = strcmp(direction, "+") == 0 ? 1 : -1;
		// 2 = relative mode 2
		KBD_OnMainActionEx(999, 0x40 + val, -1, 2, GetMainHwnd(), nullptr);
	};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		// 0 = absolute
		KBD_OnMainActionEx(999, value, -1, 0, GetMainHwnd(), nullptr);
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class UndoProcessor : public OscProcessor
{
public:
	UndoProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Undo_DoUndo2(ReaperUtils::GetProject());
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class RedoProcessor : public OscProcessor
{
public:
	RedoProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path) noexcept override
	{
		Undo_DoRedo2(ReaperUtils::GetProject());
	};

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class CursorProcessor : public OscProcessor
{
public:
	CursorProcessor(Model& aModel) : OscProcessor(aModel) {};

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		CSurf_OnArrow(value, 0);
	};

	void Process(std::deque<std::string>& path) noexcept override {};
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

class RefreshProcessor : public OscProcessor
{
public:
	RefreshProcessor(Model& aModel) : OscProcessor(aModel) {};

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

	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

#endif /* _DBM_TRANSPORTPROCESSOR_H_ */
