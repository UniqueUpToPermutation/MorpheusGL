#pragma once

#include <engine/core.hpp>

namespace Morpheus {
	class Updater : public IInitializable {
	private:
		double mLastTick;
		NodeHandle mHandle;
		bool bFirstTick;

	public:
		Updater();

		void init(Node node) override;

		void restartClock();
		void updateChildren();

		inline NodeHandle handle() const {
			return mHandle;
		}

		friend class Engine;
	};
	SET_NODE_ENUM(Updater, UPDATER);
}