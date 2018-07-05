// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ClipProcessor.h"


ClipProcessor::ClipProcessor(Model *aModel) : OscProcessor(aModel)
{
	// Intentionally empty
}


void ClipProcessor::Process(std::string command, std::deque<std::string> &path, int value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = this->GetProject();
	const char *cmd = path[1].c_str();

	if (std::strcmp(cmd, "start") == 0)
	{
		if (CountSelectedMediaItems(project) > 0)
		{
			MediaItem *item = GetSelectedMediaItem(project, 0);
			double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
			int timesig, denomOut;
			double startBPM;
			TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
			itemStart = value * 60 / startBPM;
			SetMediaItemInfo_Value(item, "D_POSITION", itemStart);
		}
	}
	else if (std::strcmp(cmd, "end") == 0)
	{
		if (CountSelectedMediaItems(project) > 0)
		{
			MediaItem *item = GetSelectedMediaItem(project, 0);
			double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
			double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
			int timesig, denomOut;
			double startBPM;
			TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &startBPM);
			itemEnd = value * 60 / startBPM;
			SetMediaItemInfo_Value(item, "D_LENGTH", itemEnd - itemStart);
		}
	}
	else if (std::strcmp(cmd, "loopStart") == 0)
	{
		double loopStart, loopEnd;
		GetSet_LoopTimeRange2(project, 0, 0, &loopStart, &loopEnd, 0);
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, loopStart, &timesig, &denomOut, &startBPM);
		loopStart = value * 60 / startBPM;
		GetSet_LoopTimeRange2(project, 1, 0, &loopStart, &loopEnd, 0);
	}
	else if (std::strcmp(cmd, "loopEnd") == 0)
	{
		double loopStart, loopEnd;
		GetSet_LoopTimeRange2(project, 0, 0, &loopStart, &loopEnd, 0);
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, loopEnd, &timesig, &denomOut, &startBPM);
		loopEnd = value * 60 / startBPM;
		GetSet_LoopTimeRange2(project, 1, 0, &loopStart, &loopEnd, 0);
	}
}


void ClipProcessor::Process(std::string command, std::deque<std::string> &path, std::string value)
{
	if (path.size() < 2)
		return;

	ReaProject *project = this->GetProject();
	const char *cmd = path[1].c_str();

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfClip(project, value);
	}
}


void ClipProcessor::SetColorOfClip(ReaProject *project, std::string value)
{
	MediaItem *item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	std::cmatch result;
	if (!std::regex_search(value.c_str(), result, colorPattern))
		return;
	int red = std::atoi(result.str(1).c_str());
	int green = std::atoi(result.str(1).c_str());
	int blue = std::atoi(result.str(1).c_str());

	Undo_BeginBlock2(project);
	SetMediaItemInfo_Value(item, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);
	UpdateItemInProject(item);
	Undo_EndBlock2(project, "Set clip color", 0);
}
