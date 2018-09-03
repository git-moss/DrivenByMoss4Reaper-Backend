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
	this->tracks.reserve(this->trackBankSize);
	for (int i = 0; i < this->trackBankSize; i++)
		this->tracks.push_back(new Track(this->sendBankSize));
}
