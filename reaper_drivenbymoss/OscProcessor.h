// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_OSCPROCESSOR_H_
#define _DBM_OSCPROCESSOR_H_

#include <cstring>
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
	OscProcessor(Model& aModel) : model(aModel)
	{
		// Intentionally empty
	}
	OscProcessor(const OscProcessor&) = delete;
	OscProcessor& operator=(const OscProcessor&) = delete;
	OscProcessor(OscProcessor&&) = delete;
	OscProcessor& operator=(OscProcessor&&) = delete;
	virtual ~OscProcessor() {};

	virtual void Process(std::deque<std::string>& path) = 0;

	virtual void Process(std::deque<std::string>& path, const std::string& value) = 0;

	virtual void Process(std::deque<std::string>& path, const std::vector<std::string>& values) = 0;

	virtual void Process(std::deque<std::string>& path, int value)
	{
		if (value == 1)
			this->Process(path);
	};

	virtual void Process(std::deque<std::string>& path, double value) = 0;

	virtual void Process(std::deque<std::string>& path, float value)
	{
		this->Process(path, static_cast<double>(value));
	};

protected:

	/** Start playback. */
	static const int    TRANSPORT_PLAY = 1007;
	/** Stop playback. */
	static const int    TRANSPORT_STOP = 1016;
	/** Record. */
	static const int    TRANSPORT_RECORD = 1013;
	/** Toggle repeat. */
	static const int    TRANSPORT_REPEAT = 1068;

	/** Tempo: Increase current project tempo 0.1 BPM. */
	static const int    TEMPO_INC_SLOW = 41137;
	/** Tempo: Increase current project tempo 01 BPM. */
	static const int    TEMPO_INC = 41129;
	/** Tempo: Decrease current project tempo 0.1 BPM. */
	static const int    TEMPO_DEC_SLOW = 41138;
	/** Tempo: Decrease current project tempo 01 BPM. */
	static const int    TEMPO_DEC = 41130;

	/** Previous project tab. */
	static const int    PROJECT_TAB_PREVIOUS = 40862;
	/** Next project tab. */
	static const int    PROJECT_TAB_NEXT = 40861;
	/** Transport: Tap tempo. */
	static const int    TRANSPORT_TAP_TEMPO = 1134;

	/** Item: Remove items. */
	static const int    REMOVE_ITEMS = 40006;
	/** Item: Duplicate items. */
	static const int    DUPLICATE_ITEMS = 41295;
	/** Item: Glue items. */
	static const int    GLUE_ITEMS = 41588;

	/** Undo. */
	static const int    EDIT_UNDO = 40029;
	/** Redo. */
	static const int    EDIT_REDO = 40030;
	/** Record: Set record mode to normal. */
	static const int    RECORD_MODE_NORMAL = 40252;
	/** Record: Set record mode to selected item auto-punch. */
	static const int    RECORD_MODE_PUNCH_ITEMS = 40253;
	/** Record: Set record mode to time selection auto-punch. */
	static const int    RECORD_MODE_AUTO_PUNCH = 40076;

	/** Pre-roll: Toggle pre-roll on record. */
	static const int    RECORD_PREROLL = 41819;
	/** Options: Toggle metronome. */
	static const int    TOGGLE_METRONOME = 40364;
	/** Options: Enable metronome. */
	static const int    ENABLE_METRONOME = 41745;
	/** Options: Disable metronome. */
	static const int    DISABLE_METRONOME = 41746;

	/** Track: Insert new track. */
	static const int    INSERT_NEW_TRACK = 40001;
	/** Track: Insert new track at end of mixer. */
	static const int    INSERT_NEW_TRACK_AT_END = 41147;
	/** Track: Insert track from template.... */
	static const int    INSERT_NEW_TRACK_FROM_TEMPLATE = 46000;
	/** Track: Duplicate tracks. */
	static const int    DUPLICATE_TRACKS = 40062;

	/** Item: Open in built in midi editor. */
	static const int    OPEN_IN_BUILT_IN_MIDI_EDITOR = 40153;
	/** Load the 1st window set. */
	static const int    LOAD_WINDOW_SET_1 = 40454;
	/** Load the 2nd window set. */
	static const int    LOAD_WINDOW_SET_2 = 40455;
	/** Load the 3rd window set. */
	static const int    LOAD_WINDOW_SET_3 = 40456;

	/** Envelope: Toggle show all active envelopes. */
	static const int    SHOW_ALL_ACTIVE_ENVELOPES = 40926;
	/** View: Toggle mixer visible. */
	static const int    TOGGLE_MIXER_VISIBLE = 40078;
	/** View: Toggle show MIDI editor windows. */
	static const int    TOGGLE_SHOW_MIDI_EDITOR_WINDOWS = 40716;
	/** View: Show track manager window. */
	static const int    SHOW_TRACK_MANAGER_WINDOW = 40906;
	/** Toggle fullscreen. */
	static const int    TOGGLE_FULLSCREEN = 40346;
	/** Media explorer: Show/hide media explorer. */
	static const int    TOGGLE_MEDIA_EXPLORER = 50124;

	/** Mixer: Toggle show FX inserts if space available. */
	static const int    TOGGLE_FX_INSERTS = 40549;
	/** Mixer: Toggle show sends if space available. */
	static const int    TOGGLE_FX_SENDS = 40557;
	/** Mixer: Toggle show FX parameters if space available. */
	static const int    TOGGLE_FX_PARAMETERS = 40910;

	/** Select all notes in the midi editor. */
	static const int    MIDI_SELECT_ALL_NOTES = 40003;
	/** Quantize all notes in the midi editor. */
	static const int    MIDI_QUANTIZE_SELECTED_NOTES = 40728;

	/** Item: Dynamic split items... */
	static const int    DYNAMIC_SPLIT = 40760;
	/** Item: Unselect all items */
	static const int    UNSELECT_ALL_ITEMS = 40289;

	/** Track : Mute Tracks. */
	static const int    MUTE_TRACKS = 40730;
	/** Track : Unmute Tracks. */
	static const int    UNMUTE_TRACKS = 40731;

	/** Track: Disable MIDI input quantize for selected tracks */
	static const int    DISABLE_MIDI_INPUT_QUANTIZE = 42064;
	/** Track: Set MIDI input quantize to grid for selected tracks */
	static const int    ENABLE_MIDI_INPUT_QUANTIZE = 42063;
	/** Track: Set MIDI input quantize to 1/4 for selected tracks */
	static const int    SET_MIDI_INPUT_QUANTIZE_1_4 = 42043;
	/** Track: Set MIDI input quantize to 1/8 for selected tracks */
	static const int    SET_MIDI_INPUT_QUANTIZE_1_8 = 42041;
	/** Track: Set MIDI input quantize to 1/16 for selected tracks */
	static const int    SET_MIDI_INPUT_QUANTIZE_1_16 = 42039;
	/** Track: Set MIDI input quantize to 1/32 for selected tracks */
	static const int    SET_MIDI_INPUT_QUANTIZE_1_32 = 42037;

	/** Track: Set all FX offline for selected tracks. */
	static const int    SET_ALL_FX_OFFLINE = 40535;
	/** Track: Set all FX online for selected tracks. */
	static const int    SET_ALL_FX_ONLINE = 40536;

	/** Track: Lock track controls. */
	static const int    LOCK_TRACK_CONTROLS = 41312;
	/** Track: Unock track controls. */
	static const int    UNLOCK_TRACK_CONTROLS = 41313;

	/** Track: Go to next track. */
	static const int    GO_TO_NEXT_TRACK = 40285;
	/** Track: Go to previous track. */
	static const int    GO_TO_PREV_TRACK = 40286;
	/** Track: Vertical scroll selected tracks into view. */
	static const int    VERTICAL_SCROLL_TRACK_INTO_VIEW = 40913;

	/** View: Zoom out horizontal. */
	static const int    ZOOM_OUT_HORIZ = 1011;
	/** View: Zoom in horizontal. */
	static const int    ZOOM_IN_HORIZ = 1012;
	/** View: Zoom out vertical. */
	static const int    ZOOM_OUT_VERT = 40112;
	/** View: Zoom in vertical. */
	static const int    ZOOM_IN_VERT = 40111;

	const std::regex colorPattern{ "RGB\\((\\d+),(\\d+),(\\d+)\\)" };

	Model& model;


	const char* SafeGet(std::deque<std::string>& path, const int index) noexcept
	{
		try
		{
			return path.at(index).c_str();
		}
		catch (...)
		{
			return "";
		}
	}


	const char* SafeGet(std::vector<std::string>& path, const int index) noexcept
	{
		try
		{
			return path.at(index).c_str();
		}
		catch (...)
		{
			return "";
		}
	}


	std::vector<std::string> SplitString(const std::string& value, char delimiter) const noexcept
	{
		std::vector<std::string> result{};
		try
		{
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
		}
		catch (...)
		{
			// Ignore
		}
		return result;
	}


	bool IsNumber(const std::string& s) const
	{
		return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
	}
};

#endif /* _DBM_OSCPROCESSOR_H_ */