#pragma once

#include "core.hpp"

namespace Morpheus {
	class Engine {
	private:
		GLFWwindow* mWindow;
		nlohmann::json mConfig;
		NodeGraph mGraph;
		ContentManager* mContent;
		NodeHandle mHandle;
		bool bValid;

	public:
		inline nlohmann::json& config() { return mConfig; }
		inline NodeGraph& graph() { return mGraph; }
		inline NodeHandle handle() const { return mHandle; }
		inline ContentManager& content() { return *mContent; }
		inline GLFWwindow* window() { return mWindow; }

		Engine();

		Error startup(const std::string& configPath);
		void update();
		void render();
		void shutdown();
		void exit();
		bool valid() const;
	};

	Engine& engine();
	inline NodeGraph& graph() {
		return engine().graph();
	}
	inline ContentManager& content() {
		return engine().content();
	}
}