// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "reaper_plugin_functions.h"
#include "Collectors.h"
#include "ReaperUtils.h"
#include "Marker.h"


/**
 * Constructor.
 */
Marker::Marker()
{
	// Intentionally empty
}


/**
 * Destructor.
 */
Marker::~Marker()
{
	// Intentionally empty
}


/**
 * Collect the (changed) marker data.
 *
 * @param ss The stream where to append the formatted data
 * @param project The current Reaper project
 * @param markerIndex The index of the marker
 * @param markerCount The number of all markers
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Marker::CollectData(std::stringstream &ss, ReaProject *project, int markerIndex, int markerCount, const bool &dump)
{
	std::stringstream das;
	das << "/marker/" << markerIndex << "/";
	std::string markerAddress = das.str();

	// Marker exists flag and number of markers
	const bool exists = markerIndex < markerCount ? 1 : 0;
	Collectors::CollectIntValue(ss, (markerAddress + "exists").c_str(), this->exists, exists, dump);
	Collectors::CollectIntValue(ss, (markerAddress + "number").c_str(), this->number, markerIndex, dump);

	const char* name;
	bool isRegion;
	double markerPos;
	double regionEnd;
	int markerRegionIndexNumber;
	int markerColor;
	int result = exists ? EnumProjectMarkers3(project, markerIndex, &isRegion, &markerPos, &regionEnd, &name, &markerRegionIndexNumber, &markerColor) : 0;

	// Marker name
	Collectors::CollectStringValue(ss, (markerAddress + "name").c_str(), this->name, result ? name : "", dump);

	// Marker color
	int red = 0, green = 0, blue = 0;
	if (exists)
		ColorFromNative(markerColor & 0xFEFFFFFF, &red, &green, &blue);
	Collectors::CollectStringValue(ss, (markerAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);
}
