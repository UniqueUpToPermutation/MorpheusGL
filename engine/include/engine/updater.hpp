#pragma once

#include <engine/core.hpp>

namespace Morpheus {
	class Updater : public INodeOwner {
	private:
		double mLastTick;
		bool bFirstTick;

	public:
		Updater();

		void init() override;

		void restartClock();
		void updateChildren();

		friend class Engine;
	};
	SET_NODE_ENUM(Updater, UPDATER);
}