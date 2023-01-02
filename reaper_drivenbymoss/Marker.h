// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2023
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_MARKER_H_
#define _DBM_MARKER_H_

#include <string>
#include <vector>

#include "ReaperUtils.h"


/**
 * Encapsulates Reaper C functions for markers.
 */
class Marker
{
public:
	int exists{ 0 };
	int number{ 0 };
	std::string name;
	std::string color;


	Marker() noexcept;
	Marker(const Marker&) = delete;
	Marker& operator=(const Marker&) = delete;
	Marker(Marker&&) = delete;
	Marker& operator=(Marker&&) = delete;
	virtual ~Marker();

	void CollectData(std::ostringstream &ss, ReaProject *project, const char *tag, int markerIndex, int markerID, const bool &dump);

	static std::vector<int> GetMarkers(ReaProject *project) noexcept;
	static std::vector<int> GetRegions(ReaProject *project) noexcept;
};

#endif /* _DBM_MARKER_H_ */