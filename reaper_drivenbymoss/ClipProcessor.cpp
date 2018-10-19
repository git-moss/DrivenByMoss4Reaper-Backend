// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ClipProcessor.h"
#include "ReaperUtils.h"


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

	ReaProject *project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem *item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

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
			Main_OnCommandEx(actionID, 0, ReaperUtils::GetProject());

		// Item: Glue items
		Main_OnCommandEx(41588, 0, project);

		Undo_EndBlock2(project, "Duplicate content of clip", 0);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		int pitch = std::atoi(path.at(1).c_str());
		const char *noteCmd = path.at(2).c_str();

		// Clear all notes with a specific pitch
		if (std::strcmp(noteCmd, "clear") == 0)
		{
			this->ClearNotes(project, item, pitch);
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

	ReaProject *project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem *item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	const char *cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "start") == 0)
	{
		double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
		itemStart = value * 60.0 / startBPM;
		SetMediaItemInfo_Value(item, "D_POSITION", itemStart);
		return;
	}

	if (std::strcmp(cmd, "end") == 0)
	{
		const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &startBPM);
		itemEnd = value * 60.0 / startBPM;
		SetMediaItemInfo_Value(item, "D_LENGTH", itemEnd - itemStart);
		return;
	}

	if (std::strcmp(cmd, "transpose") == 0)
	{
		this->TransposeClip(project, item, (int)value);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		int pitch = std::atoi(path.at(1).c_str());
		const char *noteCmd = path.at(2).c_str();

		if (std::strcmp(noteCmd, "clear") == 0)
		{
			MediaItem_Take *take = GetActiveTake(item);
			if (take == nullptr)
				return;
			const double ppqPosClipStart = MIDI_GetPPQPosFromProjQN(take, 0);
			const double ppqPosStart = MIDI_GetPPQPosFromProjQN(take, value) - ppqPosClipStart;
			this->ClearNote(project, item, pitch, ppqPosStart);
			return;
		}
		return;
	}

	if (std::strcmp(cmd, "loop") == 0)
	{
		SetMediaItemInfo_Value(item, "B_LOOPSRC", value > 0);
		return;
	}
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::string command, std::deque<std::string> &path, const std::string &value)
{
	if (path.empty())
		return;

	ReaProject *project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem *item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	const char *cmd = path.at(0).c_str();

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfClip(project, item, value);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
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

		// Subtract the start of the clip
		const double ppqPosClipStart = MIDI_GetPPQPosFromProjQN(take, 0);
		const double ppqPosStart = MIDI_GetPPQPosFromProjQN(take, pos) - ppqPosClipStart;
		const double ppqPosEnd = MIDI_GetPPQPosFromProjQN(take, pos + length) - ppqPosClipStart;

		if (std::strcmp(noteCmd, "toggle") == 0)
		{
			if (!ClearNote(project, item, pitch, ppqPosStart))
			{
				MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, 0, pitch, velocity, nullptr);
				UpdateItemInProject(item);
				Undo_OnStateChange_Item(project, "Insert note", item);
			}
			return;
		}

		if (std::strcmp(noteCmd, "set") == 0)
		{
			MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, 0, pitch, velocity, nullptr);
			UpdateItemInProject(item);
			Undo_OnStateChange_Item(project, "Insert note", item);
			return;
		}

		return;
	}
}


/**
 * Set the color of a clip.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param value The encoded RGB value, e.g. RGB(red,green,blue)
 */
void ClipProcessor::SetColorOfClip(ReaProject *project, MediaItem *item, std::string value)
{
	std::cmatch result{};
	if (!std::regex_search(value.c_str(), result, colorPattern))
		return;
	int red = std::atoi(result.str(1).c_str());
	int green = std::atoi(result.str(2).c_str());
	int blue = std::atoi(result.str(3).c_str());

	SetMediaItemInfo_Value(item, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);

	int takes = CountTakes(item);
	for (int i = 0; i < takes; i++)
	{
		MediaItem_Take *take = GetTake(item, i);
		if (take)
			SetMediaItemTakeInfo_Value(take, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);
	}

	UpdateItemInProject(item);
	Undo_OnStateChange_Item(project, "Set clip color", item);
}


/**
 * Transpose all notes in the given media item.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param transpose The value to transpose up or down (negative)
 */
void ClipProcessor::TransposeClip(ReaProject *project, MediaItem *item, int transpose)
{
	int takes = CountTakes(item);
	if (takes == 0)
		return;

	PreventUIRefresh(1);

	int noteCount;
	for (int i = 0; i < takes; i++)
	{
		MediaItem_Take *take = GetTake(item, i);
		if (take && TakeIsMIDI(take) && MIDI_CountEvts(take, &noteCount, nullptr, nullptr))
		{
			for (int n = 0; n < noteCount; n++)
			{
				int pitch;
				MIDI_GetNote(take, n, nullptr, nullptr, nullptr, nullptr, nullptr, &pitch, nullptr);
				pitch += transpose;
				MIDI_SetNote(take, n, nullptr, nullptr, nullptr, nullptr, nullptr, &pitch, nullptr, nullptr);
			}
		}
	}

	UpdateItemInProject(item);
	PreventUIRefresh(-1);
	Undo_OnStateChange_Item(project, "Transpose selected midi item notes", item);
}


/**
 * Delete all notes of a certain pitch.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param pitch The pitch of the notes to delete
 */
void ClipProcessor::ClearNotes(ReaProject *project, MediaItem *item, int pitch)
{
	MediaItem_Take *take = GetActiveTake(item);
	if (take == nullptr || !TakeIsMIDI(take))
		return;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return;

	PreventUIRefresh(1);

	int notePitch;
	for (int id = 0; id < noteCount; id++)
	{
		MIDI_GetNote(take, id, nullptr, nullptr, nullptr, nullptr, nullptr, &notePitch, nullptr);
		if (pitch == notePitch)
			MIDI_DeleteNote(take, id);
	}

	UpdateItemInProject(item);
	Undo_OnStateChange_Item(project, "Delete notes", item);

	PreventUIRefresh(-1);
}


/**
 * Delete a note of a certain pitch and position.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param pitch The pitch of the note to delete
 * @param position The position of the note to delete
 * @return True if note was found and deleted
 */
bool ClipProcessor::ClearNote(ReaProject *project, MediaItem *item, int pitch, double position)
{
	MediaItem_Take *take = GetActiveTake(item);
	if (take == nullptr || !TakeIsMIDI(take))
		return false;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return false;

	bool found{ false };
	int notePitch;
	double startppqpos{ -1 };
	for (int id = 0; id < noteCount; id++)
	{
		MIDI_GetNote(take, id, nullptr, nullptr, &startppqpos, nullptr, nullptr, &notePitch, nullptr);
		if (pitch == notePitch && std::abs(startppqpos - position) < 0.0001)
		{
			MIDI_DeleteNote(take, id);
			found = true;
			break;
		}
	}

	if (found)
	{
		UpdateItemInProject(item);
		Undo_OnStateChange_Item(project, "Delete note", item);
	}
	return found;
}
