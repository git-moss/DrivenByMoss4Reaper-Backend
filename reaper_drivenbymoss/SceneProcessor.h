// Written by Jürgen Moßgraber - mossgrabers.de
// (c) 2018-2025
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
	SceneProcessor(Model &model);

	void Process(std::deque<std::string> &path) override;

	void Process(std::deque<std::string>& path, const std::string& value) noexcept override;
	void Process(std::deque<std::string>& path, const std::vector<std::string>& values) noexcept override {};
	void Process(std::deque<std::string>& path, double value) noexcept override {};

	static void DuplicateScene(ReaProject* project, const int sceneID, const std::unique_ptr<Marker>& scene);
};

#endif /* _DBM_SCENEPROCESSOR_H_ */
