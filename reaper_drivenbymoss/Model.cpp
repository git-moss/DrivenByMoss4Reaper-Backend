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
