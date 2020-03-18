// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2020
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "WrapperGSL.h"
#include "Model.h"


/**
 * Constructor.
 */
Model::Model(FunctionExecutor& aFunctionExecutor) :
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
std::shared_ptr <Track> Model::GetTrack(const int index)
{
	this->tracklock.lock();
	const int diff = index - gsl::narrow_cast<int> (this->tracks.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->tracks.push_back(std::make_shared<Track>());
	}
	std::shared_ptr <Track> track = this->tracks.at(index);
	this->tracklock.unlock();
	return track;
}


/**
 * Get a marker.
 *
 * @param index The index of the marker.
 * @return The marker, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Marker> Model::GetMarker(const int index)
{
	this->markerlock.lock();
	const int diff = index - gsl::narrow_cast<int> (this->markers.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->markers.push_back(std::make_shared<Marker>());
	}
	std::shared_ptr <Marker> marker = this->markers.at(index);
	this->markerlock.unlock();
	return marker;
}


/**
 * Get a region.
 *
 * @param index The index of the region.
 * @return The region, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Marker> Model::GetRegion(const int index)
{
	this->regionlock.lock();
	const int diff = index - gsl::narrow_cast<int> (this->regions.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->regions.push_back(std::make_shared<Marker>());
	}
	std::shared_ptr <Marker> region = this->regions.at(index);
	this->regionlock.unlock();
	return region;
}


/**
 * Get a parameter.
 *
 * @param index The index of the parameter.
 * @return The parameter, if none exists at the index a new instance is created automatically
 */
std::shared_ptr <Parameter> Model::GetParameter(const int index)
{
	this->parameterlock.lock();
	const int diff = index - gsl::narrow_cast<int> (this->parameters.size()) + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->parameters.push_back(std::make_shared <Parameter>());
	}
	std::shared_ptr <Parameter> parameter = this->parameters.at(index);
	this->parameterlock.unlock();
	return parameter;
}
