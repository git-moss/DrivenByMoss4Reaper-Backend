// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2022
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_NOTEREPEATPROCESSOR_H_
#define _DBM_NOTEREPEATPROCESSOR_H_

#include "OscProcessor.h"


/**
 * Processes all commands related to note repeat.
 */
class NoteRepeatProcessor : public OscProcessor
{
public:
    static constexpr char const* MIDI_ARP_PLUGIN { "midi_arp" };
	static const int MIDI_ARP_PARAM_RATE{ 0 };
	static const int MIDI_ARP_PARAM_NOTE_LENGTH{ 1 };
	static const int MIDI_ARP_PARAM_MODE{ 2 };
	static const int MIDI_ARP_PARAM_VELOCITY{ 7 };

	NoteRepeatProcessor(Model& model);

	void Process(std::deque<std::string>& path, int value) noexcept override;
	void Process(std::deque<std::string>& path, double value) noexcept override;

	void Process(std::deque<std::string>& path) noexcept override {};
	void Process(std::deque<std::string>& path, const std::string& value) noexcept  override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};

private:
	void EnableRepeatPlugin(ReaProject* project, MediaTrack* track, bool enable) const noexcept;
	void SetParameter(ReaProject* project, MediaTrack* track, int parameterIndex, double value) const noexcept;
};

#endif /* _DBM_NOTEREPEATPROCESSOR_H_ */
