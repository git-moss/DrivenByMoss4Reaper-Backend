// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MARKERPROCESSOR_H_
#define _DBM_MARKERPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to markers.
 */
class MarkerProcessor : public OscProcessor
{
public:
	MarkerProcessor(Model &model) noexcept;

	void Process(std::deque<std::string> &path) noexcept override;
};

#endif /* _DBM_MARKERPROCESSOR_H_ */