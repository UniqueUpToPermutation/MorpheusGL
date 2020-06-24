#pragma once

#include "core.hpp"

namespace Morpheus {
	class Engine {
	private:
		GLFWwindow* window;
		nlohmann::json config_;
		NodeGraph graph_;
		ContentManager* content_;
		NodeHandle handle_;
		bool bValid;

	public:
		inline nlohmann::json& config() { return config_; }
		inline NodeGraph& graph() { return graph_; }
		inline NodeHandle handle() const { return handle_; }
		inline ContentManager& content() { return *content_; }

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