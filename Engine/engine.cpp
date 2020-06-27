#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include "engine.hpp"
#include "content.hpp"

using namespace std;

#define DEFAULT_VERSION_MAJOR 3
#define DEFAULT_VERSION_MINOR 3

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

namespace Morpheus {
	Engine* gEngine = nullptr;
	Engine& engine() { return *gEngine; }

	Engine::Engine() : window_(nullptr), bValid(false) {
		gEngine = this;
	}

	Error Engine::startup(const std::string& configPath) {

		if (!glfwInit())
		{
			Error err(ErrorCode::FAIL_GLFW_INIT);
			err.message = "GLFW failed to initialize!";
			err.source = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		// Load config
		ifstream f(configPath);
		f >> config_;

		auto glConfig = config_["opengl"];
		uint32_t majorVersion = DEFAULT_VERSION_MAJOR;
		uint32_t minorVersion = DEFAULT_VERSION_MINOR;
		glConfig["v_major"].get_to(majorVersion);
		glConfig["v_minor"].get_to(minorVersion);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
		glfwSetErrorCallback(error_callback);

		window_ = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
		if (!window_)
		{
			Error err(ErrorCode::FAIL_GLFW_WINDOW_INIT);
			err.message = "GLFW failed to create window!";
			err.source = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		glfwMakeContextCurrent(window_);
		gladLoadGL(); // Use GLAD to load necessary extensions
		glfwSwapInterval(1); // Set VSync on

		// Start building the engine graph
		auto v = graph_.addNode(this);
		handle_ = graph_.issueHandle(v);

		// Create content manager
		content_ = new ContentManager();
		graph_.addNode<ContentManager>(content_, handle_);

		// Set valid
		bValid = true;

		return Error(ErrorCode::SUCCESS);
	}

	bool Engine::valid() const {
		return !glfwWindowShouldClose(window_) && bValid;
	}

	void Engine::update() {
		glfwPollEvents(); // Update window!
	}

	void Engine::render() {
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window_);
	}

	void Engine::shutdown() {

		content_->unloadAll();
		delete content_;

		glfwDestroyWindow(window_);

		glfwTerminate();
	}
	void Engine::exit()
	{
		bValid = false;
	}
}