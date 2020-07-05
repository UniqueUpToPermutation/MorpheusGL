#pragma once

#include "core.hpp"

namespace Morpheus {
	class Updater {
	private:
		double mLastTick;
		NodeHandle mHandle;
		bool bFirstTick;

	public:
		Updater();

		void restartClock();
		void updateChildren();

		inline NodeHandle handle() const {
			return mHandle;
		}

		friend class Engine;
	};
	SET_NODE_TYPE(Updater, UPDATER);
}