#include <engine/updater.hpp>
#include <engine/engine.hpp>

#include <GLFW/glfw3.h>

namespace Morpheus {
	Updater::Updater() : INodeOwner(NodeType::UPDATER), mLastTick(0.0), bFirstTick(true) {
	}

	void Updater::init()
	{
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
		for (auto it = children(); it.valid(); it.next()) {
			it()->update(dt);
		}
	}
}