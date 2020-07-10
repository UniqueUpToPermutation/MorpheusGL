#include "updater.hpp"
#include "engine.hpp"

#include <GLFW/glfw3.h>

namespace Morpheus {
	Updater::Updater() : mLastTick(0.0), bFirstTick(true) {
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
		for (auto it = v.getOutgoingNeighbors(); it.valid(); it.next()) {
			auto desc = graph_->desc(it());
			desc->owner.getAs<IUpdatable>()->update(dt);
		}
	}
}