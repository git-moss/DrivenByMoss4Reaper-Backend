// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
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
	ClipProcessor(Model& model) noexcept;

	void Process(std::deque<std::string>& path) noexcept override;

	void Process(std::deque<std::string>& path, int value) noexcept override
	{
		Process(path, static_cast<double> (value));
	};

	void Process(std::deque<std::string>& path, double value) noexcept override;
	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;

private:
	void SetColorOfClip(ReaProject* project, MediaItem* item, const std::string& value) noexcept;
	void TransposeClip(ReaProject* project, MediaItem* clip, int transpose) noexcept;
	void ClearNotes(ReaProject* project, MediaItem* item, int channel, int pitch) noexcept;
	bool ClearNote(ReaProject* project, MediaItem* item, int channel, int pitch, double position) noexcept;
};

#endif /* _DBM_CLIPPROCESSOR_H_ */