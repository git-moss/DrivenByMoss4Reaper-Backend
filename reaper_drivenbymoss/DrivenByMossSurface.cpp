// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#include "ReaDebug.h"
#include "DrivenByMossSurface.h"
#include "de_mossgrabers_transformator_TransformatorApplication.h"

extern DrivenByMossSurface *gSurface;


/**
 * Constructor.
 */
DrivenByMossSurface::DrivenByMossSurface() noexcept : model(functionExecutor)
{
	gSurface = this;
	ReaDebug::init(&model);
}


/**
 * Destructor.
 */
DrivenByMossSurface::~DrivenByMossSurface()
{
	gSurface = nullptr;
}
