// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include <algorithm>
#include <cmath>
#include "Model.h"


/**
 * Constructor.
 */
Model::Model(FunctionExecutor &aFunctionExecutor) :
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
Track *Model::GetTrack(const int index)
{
	this->tracklock.lock();
	const int diff = index - (int) this->tracks.size() + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->tracks.push_back(new Track(this->sendBankSize));
	}
	Track *track = this->tracks.at(index);
	this->tracklock.unlock();
	return track;
}


/**
 * Get a marker.
 *
 * @param index The index of the marker.
 * @return The marker, if none exists at the index a new instance is created automatically
 */
Marker *Model::GetMarker(const int index)
{
	this->markerlock.lock();
	const int diff = index - (int)this->markers.size() + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->markers.push_back(new Marker());
	}
	Marker *marker = this->markers.at(index);
	this->markerlock.unlock();
	return marker;
}


/**
 * Get a parameter.
 *
 * @param index The index of the parameter.
 * @return The parameter, if none exists at the index a new instance is created automatically
 */
Parameter *Model::GetParameter(const int index)
{
	this->parameterlock.lock();
	const int diff = index - (int)this->parameters.size() + 1;
	if (diff > 0)
	{
		for (int i = 0; i < diff; i++)
			this->parameters.push_back(new Parameter());
	}
	Parameter *parameter = this->parameters.at(index);
	this->parameterlock.unlock();
	return parameter;
}
