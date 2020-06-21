#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core.hpp"

using namespace std;

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
			Error err(FAIL_GLFW_INIT);
			err.message = "GLFW failed to initialize!";
			err.source = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwSetErrorCallback(error_callback);

		window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
		if (!window)
		{
			Error err(FAIL_GLFW_WINDOW_INIT);
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