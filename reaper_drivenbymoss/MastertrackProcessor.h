// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MASTERTRACKPROCESSOR_H_
#define _DBM_MASTERTRACKPROCESSOR_H_

#include <string>

#include "OscProcessor.h"
#include "Model.h"


/**
 * Processes all commands related to the master track.
 */
class MastertrackProcessor : public OscProcessor
{
public:
	MastertrackProcessor(Model &model);

	void Process(std::deque<std::string> &path, int value) override;
	void Process(std::deque<std::string> &path, double value) override;
	void Process(std::deque<std::string>& path, const std::string& value) override;

private:
	void SetColorOfTrack(ReaProject* project, MediaTrack* track, std::string value);
};

#endif /* _DBM_MASTERTRACKPROCESSOR_H_ */