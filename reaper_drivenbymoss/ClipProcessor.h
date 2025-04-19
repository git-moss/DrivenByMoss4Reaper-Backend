// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_CLIPPROCESSOR_H_
#define _DBM_CLIPPROCESSOR_H_

#include "OscProcessor.h"


/**
 * Processes all commands related to clips.
 */
class ClipProcessor : public OscProcessor
{
public:
	ClipProcessor(Model& aModel);

	void Process(std::deque<std::string>& path) noexcept override;

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		Process(path, static_cast<double> (value));
	};

	void Process(std::deque<std::string>& path, double value) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};

private:
	void SetColorOfClip(ReaProject* project, MediaItem* item, const std::string& value) noexcept;
	void SetNameOfClip(ReaProject* project, MediaItem* item, const std::string& value) noexcept;
	void TransposeClip(ReaProject* project, MediaItem* clip, int transpose) noexcept;
	void ClearNotes(ReaProject* project, MediaItem* item, int channel, int pitch) noexcept;
	bool ClearNote(ReaProject* project, MediaItem* item, int channel, int pitch, double position) noexcept;
	bool ClearNotesAtPosition(ReaProject* project, MediaItem* item, int channel, double position) noexcept;
	bool MoveNoteY(ReaProject* project, MediaItem* item, int channel, int pitch, int newPitch, double position) noexcept;
	int GetNoteIndex(MediaItem_Take* take, int channel, int pitch, double position) noexcept;
};

#endif /* _DBM_CLIPPROCESSOR_H_ */
