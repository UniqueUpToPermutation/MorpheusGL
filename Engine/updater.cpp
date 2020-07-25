#include "updater.hpp"
#include "engine.hpp"


#include "cameracontroller.hpp"

#include <GLFW/glfw3.h>

namespace Morpheus {
	Updater::Updater() : mLastTick(0.0), bFirstTick(true) {
	}

	void Updater::init(Node node)
	{
		mHandle = graph()->issueHandle(node);
		restartClock();
	}

	void Updater::restartClock() {
		mLastTick = 0.0;
		glfwSetTime(mLastTick);
	}
	void Updater::updateChildren() {
		if (bFirstTick) {
			restartClock();
			bFirstTick = false;
		}

		double currentTick = glfwGetTime();
		double dt = currentTick - mLastTick;
		mLastTick = currentTick;

		// Go through all children and update them
		auto graph_ = graph();
		auto v = (*graph_)[mHandle];
		for (auto it = v.children(); it.valid(); it.next()) {
			auto desc = graph_->desc(it());
			auto updatableInterface = getInterface<IUpdatable>(*desc);
			if (updatableInterface->isEnabled())
				updatableInterface->update(dt);
		}
	}
}