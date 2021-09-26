// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2021
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_SCENEPROCESSOR_H_
#define _DBM_SCENEPROCESSOR_H_

#include <string>

#include "OscProcessor.h"


/**
 * Processes all commands related to scenes.
 */
class SceneProcessor : public OscProcessor
{
public:
	SceneProcessor(Model &model) noexcept;

	void Process(std::deque<std::string> &path) noexcept override;

	static void DuplicateScene(ReaProject* project, const int sceneID) noexcept;
};

#endif /* _DBM_SCENEPROCESSOR_H_ */