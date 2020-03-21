// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "WrapperGSL.h"
#include "Model.h"


/**
 * Constructor.
 */
Model::Model(FunctionExecutor& aFunctionExecutor)  noexcept :
	functionExecutor(aFunctionExecutor)
{
	// Intentionally empty
}


/**
 * Get a track.
 *
 * @param index The index of the track.
 * @return The track, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Track> Model::GetTrack(const int index) noexcept
{
	const std::lock_guard<std::mutex> lock(this->tracklock);

	const int diff = index - gsl::narrow_cast<int> (this->tracks.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->tracks.push_back(std::make_shared<Track>());
	}
	return this->tracks.at(index);
}


/**
 * Get a marker.
 *
 * @param index The index of the marker.
 * @return The marker, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Marker> Model::GetMarker(const int index) noexcept
{
	const std::lock_guard<std::mutex> lock(this->markerlock);

	const int diff = index - gsl::narrow_cast<int> (this->markers.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->markers.push_back(std::make_shared<Marker>());
	}
	return this->markers.at(index);
}


/**
 * Get a region.
 *
 * @param index The index of the region.
 * @return The region, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Marker> Model::GetRegion(const int index) noexcept
{
	const std::lock_guard<std::mutex> lock(this->regionlock);

	const int diff = index - gsl::narrow_cast<int> (this->regions.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->regions.push_back(std::make_shared<Marker>());
	}
	return this->regions.at(index);
}


/**
 * Get a parameter.
 *
 * @param index The index of the parameter.
 * @return The parameter, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Parameter> Model::GetParameter(const int index) noexcept
{
	const std::lock_guard<std::mutex> lock(this->parameterlock);

	const int diff = index - gsl::narrow_cast<int> (this->parameters.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->parameters.push_back(std::make_shared <Parameter>());
	}
	return this->parameters.at(index);
}
