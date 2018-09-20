// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#pragma once

#include <string>
#include <vector>


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


	Marker();
	virtual ~Marker();

	void CollectData(std::stringstream &ss, ReaProject *project, const char *tag, int markerIndex, int markerID, const bool &dump);

	static std::vector<int> GetMarkers(ReaProject *project);
	static std::vector<int> GetRegions(ReaProject *project);
};
