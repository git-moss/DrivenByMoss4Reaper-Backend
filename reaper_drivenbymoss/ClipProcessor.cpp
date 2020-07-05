// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ClipProcessor.h"
#include "ReaperUtils.h"
#include "ReaDebug.h"


/**
 * Constructor.
 *
 * @param aModel The model to share data
 */
ClipProcessor::ClipProcessor(Model& aModel) noexcept : OscProcessor(aModel)
{
	// Intentionally empty
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::deque<std::string>& path) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	const char* cmd = safeGet(path, 0);

	if (std::strcmp(cmd, "clear") == 0)
	{
		// Remove all MIDI events from the clip (midi item)
		this->ClearNotes(project, item, -1, -1);
		return;
	}

	if (std::strcmp(cmd, "duplicate") == 0)
	{
		// Item: Duplicate items
		Main_OnCommandEx(DUPLICATE_ITEMS, 0, project);
		return;
	}

	if (std::strcmp(cmd, "duplicateContent") == 0)
	{
		Undo_BeginBlock2(project);

		// Item: Duplicate items
		Main_OnCommandEx(DUPLICATE_ITEMS, 0, project);

		// SWS: Add item(s) to left of selected item(s) to selection
		const int actionID = NamedCommandLookup("_SWS_ADDLEFTITEM");
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, ReaperUtils::GetProject());

		// Item: Glue items
		Main_OnCommandEx(GLUE_ITEMS, 0, project);

		Undo_EndBlock2(project, "Duplicate content of clip", 0);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		const int pitch = std::atoi(safeGet(path, 1));
		const char* noteCmd = safeGet(path, 2);

		// Clear all notes with a specific pitch
		if (std::strcmp(noteCmd, "clear") == 0)
		{
			const int channel = atoi(safeGet(path, 3));
			this->ClearNotes(project, item, channel, pitch);
			return;
		}

		return;
	}
}


/** {@inheritDoc} */
void ClipProcessor::Process(std::deque<std::string>& path, double value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	const char* cmd = safeGet(path, 0);

	if (std::strcmp(cmd, "start") == 0)
	{
		PreventUIRefresh(1);
		double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, itemStart, &timesig, &denomOut, &startBPM);
		itemStart = value * 60.0 / startBPM;
		SetMediaItemInfo_Value(item, "D_POSITION", itemStart);
		PreventUIRefresh(-1);
		return;
	}

	if (std::strcmp(cmd, "end") == 0)
	{
		PreventUIRefresh(1);
		const double itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
		double itemEnd = itemStart + GetMediaItemInfo_Value(item, "D_LENGTH");
		int timesig, denomOut;
		double startBPM;
		TimeMap_GetTimeSigAtTime(project, itemEnd, &timesig, &denomOut, &startBPM);
		itemEnd = value * 60.0 / startBPM;
		SetMediaItemInfo_Value(item, "D_LENGTH", itemEnd - itemStart);
		PreventUIRefresh(-1);
		return;
	}

	if (std::strcmp(cmd, "transpose") == 0)
	{
		this->TransposeClip(project, item, static_cast<int>(value));
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;
		const int pitch = std::atoi(safeGet(path, 1));
		const char* noteCmd = safeGet(path, 2);

		if (std::strcmp(noteCmd, "clear") == 0)
		{
			MediaItem_Take* take = GetActiveTake(item);
			if (take == nullptr)
				return;
			const double ppqPosClipStart = MIDI_GetPPQPosFromProjQN(take, 0);
			const double ppqPosStart = MIDI_GetPPQPosFromProjQN(take, value) - ppqPosClipStart;
			const int channel = atoi(safeGet(path, 3));
			this->ClearNote(project, item, channel, pitch, ppqPosStart);
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
void ClipProcessor::Process(std::deque<std::string>& path, const std::string& value) noexcept
{
	if (path.empty())
		return;

	ReaProject* project = ReaperUtils::GetProject();
	if (CountSelectedMediaItems(project) == 0)
		return;
	MediaItem* item = GetSelectedMediaItem(project, 0);
	if (item == nullptr)
		return;

	const char* cmd = safeGet(path, 0);

	if (std::strcmp(cmd, "color") == 0)
	{
		SetColorOfClip(project, item, value);
		return;
	}

	if (std::strcmp(cmd, "note") == 0)
	{
		if (path.size() < 3)
			return;

		MediaItem_Take* take = GetActiveTake(item);
		if (take == nullptr)
			return;

		const int pitch = std::atoi(safeGet(path, 1));
		const char* noteCmd = safeGet(path, 2);

		std::vector<std::string> parts = this->SplitString(value, ' ');
		if (parts.size() != 4)
			return;

		const double pos = std::atof(safeGet(parts, 0));
		const double length = std::atof(safeGet(parts, 1));
		const int velocity = std::atoi(safeGet(parts, 2));
		const int channel = std::atoi(safeGet(parts, 3));

		// Subtract the start of the clip
		const double ppqPosClipStart = MIDI_GetPPQPosFromProjQN(take, 0);
		const double ppqPosStart = MIDI_GetPPQPosFromProjQN(take, pos) - ppqPosClipStart;
		const double ppqPosEnd = MIDI_GetPPQPosFromProjQN(take, pos + length) - ppqPosClipStart;

		if (std::strcmp(noteCmd, "toggle") == 0)
		{
			PreventUIRefresh(1);
			if (!ClearNote(project, item, channel, pitch, ppqPosStart))
			{
				MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, channel, pitch, velocity, nullptr);
				UpdateItemInProject(item);
				Undo_OnStateChange_Item(project, "Insert note", item);
			}
			PreventUIRefresh(-1);
			return;
		}

		if (std::strcmp(noteCmd, "set") == 0)
		{
			PreventUIRefresh(1);
			MIDI_InsertNote(take, false, false, ppqPosStart, ppqPosEnd, channel, pitch, velocity, nullptr);
			UpdateItemInProject(item);
			Undo_OnStateChange_Item(project, "Insert note", item);
			PreventUIRefresh(-1);
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
void ClipProcessor::SetColorOfClip(ReaProject* project, MediaItem* item, const std::string& value) noexcept
{
	int red{ 0 };
	int green{ 0 };
	int blue{ 0 };
	try
	{
		std::cmatch result{};
		if (!std::regex_search(value.c_str(), result, colorPattern))
			return;
		red = std::atoi(result.str(1).c_str());
		green = std::atoi(result.str(2).c_str());
		blue = std::atoi(result.str(3).c_str());
	}
	catch (...)
	{
		return;
	}

	PreventUIRefresh(1);
	SetMediaItemInfo_Value(item, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);

	const int takes = CountTakes(item);
	for (int i = 0; i < takes; i++)
	{
		MediaItem_Take* take = GetTake(item, i);
		if (take)
			SetMediaItemTakeInfo_Value(take, "I_CUSTOMCOLOR", ColorToNative(red, green, blue) | 0x100000);
	}

	UpdateItemInProject(item);
	Undo_OnStateChange_Item(project, "Set clip color", item);
	PreventUIRefresh(-1);
}


/**
 * Transpose all notes in the given media item.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param transpose The value to transpose up or down (negative)
 */
void ClipProcessor::TransposeClip(ReaProject* project, MediaItem* item, int transpose) noexcept
{
	const int takes = CountTakes(item);
	if (takes == 0)
		return;

	PreventUIRefresh(1);

	int noteCount{ 0 };
	for (int i = 0; i < takes; i++)
	{
		MediaItem_Take* take = GetTake(item, i);
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
	Undo_OnStateChange_Item(project, "Transpose selected midi item notes", item);
	PreventUIRefresh(-1);
}


/**
 * Delete all notes of a certain pitch.
 *
 * @param project The Reaper project
 * @param item The media item
 * @param channel The MIDI channel of the note to delete
 * @param pitch The pitch of the notes to delete
 */
void ClipProcessor::ClearNotes(ReaProject* project, MediaItem* item, int channel, int pitch) noexcept
{
	MediaItem_Take* take = GetActiveTake(item);
	if (take == nullptr || !TakeIsMIDI(take))
		return;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return;

	PreventUIRefresh(1);

	if (channel == -1 && pitch == -1)
	{
		for (int id = noteCount - 1; id >= 0; id--)
			MIDI_DeleteNote(take, id);
	}
	else
	{
		int midiChannel{ 0 };
		int notePitch{ 0 };
		for (int id = noteCount - 1; id >= 0; id--)
		{
			MIDI_GetNote(take, id, nullptr, nullptr, nullptr, nullptr, &midiChannel, &notePitch, nullptr);
			if (channel == midiChannel && pitch == notePitch)
				MIDI_DeleteNote(take, id);
		}
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
 * @param channel The MIDI channel of the note to delete
 * @param pitch The pitch of the note to delete
 * @param position The position of the note to delete
 * @return True if note was found and deleted
 */
bool ClipProcessor::ClearNote(ReaProject* project, MediaItem* item, int channel, int pitch, double position) noexcept
{
	MediaItem_Take* take = GetActiveTake(item);
	if (take == nullptr || !TakeIsMIDI(take))
		return false;

	int noteCount;
	if (MIDI_CountEvts(take, &noteCount, nullptr, nullptr) == 0)
		return false;

	PreventUIRefresh(1);

	bool found{ false };
	int midiChannel{ 0 };
	int notePitch{ 0 };
	double startppqpos{ -1 };
	for (int id = 0; id < noteCount; id++)
	{
		MIDI_GetNote(take, id, nullptr, nullptr, &startppqpos, nullptr, &midiChannel, &notePitch, nullptr);
		if (channel == midiChannel && pitch == notePitch && std::abs(startppqpos - position) < 0.0001)
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
	
	PreventUIRefresh(-1);

	return found;
}
