// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2019
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <deque>
#include <regex>

#include "Model.h"


/**
 * Interface to processing commands and executing them on Reaper.
 */
class OscProcessor
{
public:
	OscProcessor(Model &aModel) : model(aModel)
	{
		// Intentionally empty
	}

	virtual ~OscProcessor() {};

	virtual void Process(std::deque<std::string> &path) {};

	virtual void Process(std::deque<std::string> &path, const std::string &value) {};

	virtual void Process(std::deque<std::string> &path, int value)
	{
		if (value == 1)
			this->Process(path);
	};

	virtual void Process(std::deque<std::string> &path, double value) {};

	virtual void Process(std::deque<std::string> &path, float value)
	{
		this->Process(path, static_cast<double>(value));
	};

protected:
	/** Previous project tab. */
	const int    PROJECT_TAB_PREVIOUS = 40862;
	/** Next project tab. */
	const int    PROJECT_TAB_NEXT = 40861;
	/** Transport: Tap tempo. */
	const int    TRANSPORT_TAP_TEMPO = 1134;

	/** Item: Remove items. */
	const int    REMOVE_ITEMS = 40006;
	/** Item: Duplicate items. */
	const int    DUPLICATE_ITEMS = 41295;
	/** Item: Glue items. */
	const int    GLUE_ITEMS = 41588;

	/** Undo. */
	const int    EDIT_UNDO = 40029;
	/** Redo. */
	const int    EDIT_REDO = 40030;
	/** Record: Set record mode to normal. */
	const int    RECORD_MODE_NORMAL = 40252;
	/** Record: Set record mode to selected item auto-punch. */
	const int    RECORD_MODE_PUNCH_ITEMS = 40253;
	/** Record: Set record mode to time selection auto-punch. */
	const int    RECORD_MODE_AUTO_PUNCH = 40076;

	/** Pre-roll: Toggle pre-roll on record. */
	const int    RECORD_PREROLL = 41819;
	/** Options: Toggle metronome. */
	const int    TOGGLE_METRONOME = 40364;
	/** Options: Enable metronome. */
	const int    ENABLE_METRONOME = 41745;
	/** Options: Disable metronome. */
	const int    DISABLE_METRONOME = 41746;

	/** Track: Insert new track. */
	const int    INSERT_NEW_TRACK = 40001;
	/** Track: Insert new track at end of mixer. */
	const int    INSERT_NEW_TRACK_AT_END = 41147;
	/** Track: Insert track from template.... */
	const int    INSERT_NEW_TRACK_FROM_TEMPLATE = 46000;
	/** Track: Duplicate tracks. */
	const int    DUPLICATE_TRACKS = 40062;

	/** Item: Open in built in midi editor. */
	const int    OPEN_IN_BUILT_IN_MIDI_EDITOR = 40153;
	/** Load the 1st window set. */
	const int    LOAD_WINDOW_SET_1 = 40454;
	/** Load the 2nd window set. */
	const int    LOAD_WINDOW_SET_2 = 40455;
	/** Load the 3rd window set. */
	const int    LOAD_WINDOW_SET_3 = 40456;

	/** Envelope: Toggle show all active envelopes. */
	const int    SHOW_ALL_ACTIVE_ENVELOPES = 40926;
	/** View: Toggle mixer visible. */
	const int    TOGGLE_MIXER_VISIBLE = 40078;
	/** View: Toggle show MIDI editor windows. */
	const int    TOGGLE_SHOW_MIDI_EDITOR_WINDOWS = 40716;
	/** View: Show track manager window. */
	const int    SHOW_TRACK_MANAGER_WINDOW = 40906;
	/** Toggle fullscreen. */
	const int    TOGGLE_FULLSCREEN = 40346;
	/** Media explorer: Show/hide media explorer. */
	const int    TOGGLE_MEDIA_EXPLORER = 50124;

	/** Mixer: Toggle show FX inserts if space available. */
	const int    TOGGLE_FX_INSERTS = 40549;
	/** Mixer: Toggle show sends if space available. */
	const int    TOGGLE_FX_SENDS = 40557;
	/** Mixer: Toggle show FX parameters if space available. */
	const int    TOGGLE_FX_PARAMETERS = 40910;

	/** Select all notes in the midi editor. */
	const int    MIDI_SELECT_ALL_NOTES = 40003;
	/** Quantize all notes in the midi editor. */
	const int    MIDI_QUANTIZE_SELECTED_NOTES = 40728;

	/** Item: Dynamic split items... */
	const int    DYNAMIC_SPLIT = 40760;
	/** Item: Unselect all items */
	const int    UNSELECT_ALL_ITEMS = 40289;

	/** Track : Mute Tracks. */
	const int    MUTE_TRACKS = 40730;
	/** Track : Unmute Tracks. */
	const int    UNMUTE_TRACKS = 40731;

	/** Track: Set all FX offline for selected tracks. */
	const int    SET_ALL_FX_OFFLINE = 40535;
	/** Track: Set all FX online for selected tracks. */
	const int    SET_ALL_FX_ONLINE = 40536;

	/** Track: Lock track controls. */
	const int    LOCK_TRACK_CONTROLS = 41312;
	/** Track: Unock track controls. */
	const int    UNLOCK_TRACK_CONTROLS = 41313;

	/** Track: Go to next track. */
	const int    GO_TO_NEXT_TRACK = 40285;
	/** Track: Go to previous track. */
	const int    GO_TO_PREV_TRACK = 40286;

	/** View: Zoom out horizontal. */
	const int    ZOOM_OUT_HORIZ = 1011;
	/** View: Zoom in horizontal. */
	const int    ZOOM_IN_HORIZ = 1012;
	/** View: Zoom out vertical. */
	const int    ZOOM_OUT_VERT = 40112;
	/** View: Zoom in vertical. */
	const int    ZOOM_IN_VERT = 40111;

	/** SWS/S&M: Toggle show FX chain windows for selected tracks. */
	const char * SHOW_FX_CHAIN_WINDOWS = "_S&M_TOGLFXCHAIN";
	/** SWS: Add item(s) to left of selected item(s) to selection. */
	const char * ADD_LEFT_ITEM_TO_SELECTION = "_SWS_ADDLEFTITEM";

	/** SWS : Bypass FX on selected tracks. */
	const char * BYPASS_ALL_FX_ON_SELECTED_TRACKS = "_S&M_FXBYPALL2";
	/** SWS/S&M: Unbypass all FX for selected tracks. */
	const char * UNBYPASS_ALL_FX_ON_SELECTED_TRACKS = "_S&M_FXBYPALL3";

	/** SWS: Mute all sends from selected track(s). */
	const char * MUTE_ALL_SENDS_ON_SELECTED_TRACKS = "_SWS_MUTESENDS";
	/** SWS: Unmute all sends from selected track(s). */
	const char * UNMUTE_ALL_SENDS_ON_SELECTED_TRACKS = "_SWS_UNMUTESENDS";

	/** SWS: Mute all receives for selected track(s). */
	const char * MUTE_ALL_RECEIVES_ON_SELECTED_TRACKS = "_SWS_MUTERECVS";
	/** SWS: Unmute all receives for selected track(s). */
	const char * UNMUTE_ALL_RECEIVES_ON_SELECTED_TRACKS = "_SWS_UNMUTERECVS";


	const std::regex colorPattern{ "RGB\\((\\d+),(\\d+),(\\d+)\\)" };

	Model &model;


	void ExecuteActionEx(ReaProject *project, const char *action) const
	{
		const int actionID = NamedCommandLookup(action);
		if (actionID > 0)
			Main_OnCommandEx(actionID, 0, project);
	}


	std::vector<std::string> SplitString(const std::string &value, char delimiter) const
	{
		std::vector<std::string> result{};
		std::string str = value;
		size_t index;
		while ((index = str.find(delimiter)) != std::string::npos)
		{
			std::string part = str.substr(0, index);
			result.push_back(part);
			str = str.substr(index + 1, str.length() - index);
		}
		if (str.length() > 0)
			result.push_back(str);
		return result;
	}
};
