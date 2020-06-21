#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>

#include "core.hpp"

using namespace std;

#define DEFAULT_VERSION_MAJOR 3
#define DEFAULT_VERSION_MINOR 3

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

namespace Morpheus {
	Engine::Engine() : window(nullptr), bValid(false) {
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

		window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
		if (!window)
		{
			Error err(ErrorCode::FAIL_GLFW_WINDOW_INIT);
			err.message = "GLFW failed to create window!";
			err.source = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		glfwMakeContextCurrent(window);
		gladLoadGL(); // Use GLAD to load necessary extensions
		glfwSwapInterval(1); // Set VSync on

		// Set valid
		bValid = true;

		return Error(ErrorCode::SUCCESS);
	}

	bool Engine::valid() const {
		return !glfwWindowShouldClose(window) && bValid;
	}

	void Engine::update() {
		glfwPollEvents(); // Update window!
	}

	void Engine::render() {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window);
	}

	void Engine::shutdown() {

		glfwDestroyWindow(window);

		glfwTerminate();
	}
}