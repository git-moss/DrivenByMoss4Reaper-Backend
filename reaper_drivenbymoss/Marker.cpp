// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "Collectors.h"
#include "Marker.h"


/**
 * Constructor.
 */
Marker::Marker() noexcept
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
void Marker::CollectData(std::ostringstream& ss, ReaProject* project, const char* tag, int markerIndex, int markerID, const bool& dump)
{
	std::ostringstream das;
	das << "/" << tag << "/" << markerIndex << "/";
	const std::string markerAddress = das.str();

	// Marker exists flag and number of markers
	this->exists = Collectors::CollectIntValue(ss, (markerAddress + "exists").c_str(), this->exists, true, dump);
	this->number = Collectors::CollectIntValue(ss, (markerAddress + "number").c_str(), this->number, markerIndex, dump);

	const char* name;
	double pos;
	double end;
	const int result = EnumProjectMarkers3(project, markerID, nullptr, &pos, &end, &name, &this->markerOrRegionIndex, &this->colorNumber);

	// Marker name
	const std::string newName = result ? name : "";
	this->name = Collectors::CollectStringValue(ss, (markerAddress + "name").c_str(), this->name, newName, dump);

	// Position info
	this->position = Collectors::CollectDoubleValue(ss, (markerAddress + "position").c_str(), this->position, pos, dump);
	this->endPosition = Collectors::CollectDoubleValue(ss, (markerAddress + "endPosition").c_str(), this->endPosition, end, dump);

	// Marker color
	int red, green, blue;
	ColorFromNative(this->colorNumber & 0xFEFFFFFF, &red, &green, &blue);
	this->color = Collectors::CollectStringValue(ss, (markerAddress + "color").c_str(), this->color, Collectors::FormatColor(red, green, blue).c_str(), dump);
}


/**
 * Get all markers which are not regions.
 *
 * @return The indices of the markers
 */
std::vector<int> Marker::GetMarkers(ReaProject* project) noexcept
{
	std::vector<int> markers;
	const int count = CountProjectMarkers(project, nullptr, nullptr);
	bool isRegion{ false };
	for (int index = 0; index < count; index++)
	{
		if (EnumProjectMarkers2(project, index, &isRegion, nullptr, nullptr, nullptr, nullptr) && !isRegion)
		{
			try
			{
				markers.push_back(index);
			}
			catch (...)
			{
				// Ignore
			}
		}
	}
	return markers;
}


/**
 * Get all regions markers.
 *
 * @return The indices of the region markers
 */
std::vector<int> Marker::GetRegions(ReaProject* project) noexcept
{
	std::vector<int> regions;
	const int count = CountProjectMarkers(project, nullptr, nullptr);
	bool isRegion{ false };
	for (int index = 0; index < count; index++)
	{
		if (EnumProjectMarkers2(project, index, &isRegion, nullptr, nullptr, nullptr, nullptr) && isRegion)
		{
			try
			{
				regions.push_back(index);
			}
			catch (...)
			{
				// Ignore
			}
		}
	}
	return regions;
}
