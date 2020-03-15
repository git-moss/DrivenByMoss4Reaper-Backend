// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "Collectors.h"
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
 * @param tag The text to use for the id string
 * @param markerIndex The index of the marker
 * @param markerID The ID of the marker
 * @param dump If true all data is collected not only the changed one since the last call
 */
void Marker::CollectData(std::stringstream &ss, ReaProject *project, const char *tag, int markerIndex, int markerID, const bool &dump)
{
	std::stringstream das;
	das << "/" << tag << "/" << markerIndex << "/";
	std::string markerAddress = das.str();

	// Marker exists flag and number of markers
	this->exists = Collectors::CollectIntValue(ss, (markerAddress + "exists").c_str(), this->exists, true, dump);
	this->number = Collectors::CollectIntValue(ss, (markerAddress + "number").c_str(), this->number, markerIndex, dump);

	const char* name;
	int markerColor;
	int result = EnumProjectMarkers3(project, markerID, nullptr, nullptr, nullptr, &name, nullptr, &markerColor);

	// Marker name
	this->name = Collectors::CollectStringValue(ss, (markerAddress + "name").c_str(), this->name, result ? name : "", dump);

	// Marker color
	int red, green, blue;
	ColorFromNative(markerColor & 0xFEFFFFFF, &red, &green, &blue);
	this->color = Collectors::CollectStringValue(ss, (markerAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


std::vector<int> Marker::GetMarkers(ReaProject *project)
{
	std::vector<int> markers;
	int count = CountProjectMarkers(project, nullptr, nullptr);
	bool isRegion;
	for (int index = 0; index < count; index++)
	{
		if (EnumProjectMarkers2(project, index, &isRegion, nullptr, nullptr, nullptr, nullptr) && !isRegion)
			markers.push_back(index);
	}
	return markers;
}


std::vector<int> Marker::GetRegions(ReaProject *project)
{
	std::vector<int> regions;
	int count = CountProjectMarkers(project, nullptr, nullptr);
	bool isRegion;
	for (int index = 0; index < count; index++)
	{
		if (EnumProjectMarkers2(project, index, &isRegion, nullptr, nullptr, nullptr, nullptr) && isRegion)
			regions.push_back(index);
	}
	return regions;
}
