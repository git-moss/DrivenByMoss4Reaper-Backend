// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
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

	explicit NoteRepeatProcessor(Model& aModel);
	NoteRepeatProcessor(const NoteRepeatProcessor&) = delete;
	NoteRepeatProcessor& operator=(const NoteRepeatProcessor&) = delete;
	NoteRepeatProcessor(NoteRepeatProcessor&&) = delete;
	NoteRepeatProcessor& operator=(NoteRepeatProcessor&&) = delete;
	virtual ~NoteRepeatProcessor() = default;

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
