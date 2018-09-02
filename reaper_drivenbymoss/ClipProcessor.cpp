// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ClipProcessor.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
ClipProcessor::ClipProcessor(Model &aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::string command, std::deque<std::string> &path)
{
	if (path.empty())
		return;

	ReaProject *project = this->model.GetProject();
	const char *cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		// Item: Duplicate items
		Main_OnCommandEx(41295, 0, project);
		return;
	}

	if (std::strcmp(cmd, "duplicateContent") == 0)
	{
		Undo_BeginBlock2(project);

		// Item: Duplicate items
		Main_OnCommandEx(41295, 0, project);

		// SWS: Add item(s) to left of selected item(s) to selection
		const int actionID = NamedCommandLookup("_SWS_ADDLEFTITEM");
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, this->model.GetProject());

		// Item: Glue items
		Main_OnCommandEx(41588, 0, project);

		Undo_EndBlock2(project, "Duplicate content of clip", 0);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		int note = std::atoi(path.at(1).c_str());
		const char *noteCmd = path.at(2).c_str();

		if (std::strcmp(noteCmd, "clear") == 0)
		{
			// TODO Implement clear
			return;
		}

		return;
	}
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::string command, std::deque<std::string> &path, double value)
{
	if (path.empty())
		return;

	ReaProject *project = this->model.GetProject();
	const char *cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "start") == 0)
	{
		if (CountSelectedMediaItems(project) > 0)
		{
			MediaItem *item = GetSelectedMediaItem(project, 0);
			double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
			int timesig, denomOut;
			double startBPM;
			TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
			itemStart = static_cast<double>(value) * 60.0 / startBPM;
			SetMediaItemInfo_Value(item, "D_POSITION", itemStart);
		}
		return;
	}

	if (std::strcmp(cmd, "end") == 0)
	{
		if (CountSelectedMediaItems(project) > 0)
		{
			MediaItem *item = GetSelectedMediaItem(project, 0);
			const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
			double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
			int timesig, denomOut;
			double startBPM;
			TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &startBPM);
			itemEnd = static_cast<double>(value) * 60.0 / startBPM;
			SetMediaItemInfo_Value(item, "D_LENGTH", itemEnd - itemStart);
		}
		return;
	}

	if (std::strcmp(cmd, "loopStart") == 0)
	{
		double loopStart, loopEnd;
		GetSet_LoopTimeRange2(project, 0, 0, &loopStart, &loopEnd, 0);
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, loopStart, &timesig, &denomOut, &startBPM);
		loopStart = static_cast<double>(value) * 60.0 / startBPM;
		GetSet_LoopTimeRange2(project, 1, 0, &loopStart, &loopEnd, 0);
		return;
	}

	if (std::strcmp(cmd, "loopEnd") == 0)
	{
		double loopStart, loopEnd;
		GetSet_LoopTimeRange2(project, 0, 0, &loopStart, &loopEnd, 0);
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, loopEnd, &timesig, &denomOut, &startBPM);
		loopEnd = static_cast<double>(value) * 60.0 / startBPM;
		GetSet_LoopTimeRange2(project, 1, 0, &loopStart, &loopEnd, 0);
		return;
	}

	if (std::strcmp(cmd, "transpose") == 0)
	{
		// TODO Implement transpose
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		int note = std::atoi(path.at(1).c_str());
		const char *noteCmd = path.at(2).c_str();

		if (std::strcmp(noteCmd, "clear") == 0)
		{
			// TODO Implement clear
			return;
		}

		return;
	}
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::string command, std::deque<std::string> &path, const std::string &value)
{
	if (path.empty())
		return;

	ReaProject *project = this->model.GetProject();
	const char *cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfClip(project, value);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;

		MediaItem *item = GetSelectedMediaItem(project, 0);
		if (item == nullptr)
			return;
		MediaItem_Take *take = GetActiveTake(item);
		if (take == nullptr)
			return;

		const int pitch = std::atoi(path.at(1).c_str());
		const char *noteCmd = path.at(2).c_str();

		std::vector<std::string> parts = this->SplitString(value, ' ');
		if (parts.size() != 3)
			return;

		const double pos = std::atof(parts.at(0).c_str());
		const double length = std::atof(parts.at(1).c_str());
		const int velocity = std::atoi(parts.at(2).c_str());

		const double ppqPosStart = MIDI_GetPPQPosFromProjQN(take, pos);
		const double ppqPosEnd = MIDI_GetPPQPosFromProjQN(take, pos + length);

		if (std::strcmp(noteCmd, "toggle") == 0)
		{
			// TODO Implement toggle
			MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, 0, pitch, velocity, nullptr);
			return;
		}

		if (std::strcmp(noteCmd, "set") == 0)
		{
			MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, 0, pitch, velocity, nullptr);
			return;
		}

		return;
	}
}


/**
 * Set the color of a clip.
 *
 * @param project The Reaper project
 * @param value   The encoded RGB value, e.g. RGB(red,green,blue)
 */
void ClipProcessor::SetColorOfClip(ReaProject *project, std::string value)
{
	MediaItem *item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	std::cmatch result{};
	if (!std::regex_search(value.c_str(), result, colorPattern))
		return;
	int red = std::atoi(result.str(1).c_str());
	int green = std::atoi(result.str(2).c_str());
	int blue = std::atoi(result.str(3).c_str());

	Undo_BeginBlock2(project);
	SetMediaItemInfo_Value(item, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);
	UpdateItemInProject(item);
	Undo_EndBlock2(project, "Set clip color", 0);
}
