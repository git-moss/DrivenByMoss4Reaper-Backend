// Copyright (c) 2018-2025 by Jürgen Moßgraber (www.mossgrabers.de)
// Licensed under LGPLv3 - http://www.gnu.org/licenses/lgpl-3.0.txt

#ifndef _DBM_SCENEPROCESSOR_H_
#define _DBM_SCENEPROCESSOR_H_

#include "OscProcessor.h"


/**
 * Processes all commands related to scenes.
 */
class SceneProcessor : public OscProcessor
{
public:
	SceneProcessor(Model &aModel);

	void Process(std::deque<std::string> &path) override;

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};

	static void DuplicateScene(ReaProject* project, const int sceneID, Marker* scene);
};

#endif /* _DBM_SCENEPROCESSOR_H_ */
