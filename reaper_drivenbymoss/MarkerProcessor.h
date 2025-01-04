// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MARKERPROCESSOR_H_
#define _DBM_MARKERPROCESSOR_H_

#include "OscProcessor.h"


/**
 * Processes all commands related to markers.
 */
class MarkerProcessor : public OscProcessor
{
public:
	MarkerProcessor(Model& model);

	void Process(std::deque<std::string>& path) override;

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override {};
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};
};

#endif /* _DBM_MARKERPROCESSOR_H_ */
