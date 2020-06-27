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

	Engine::Engine() : mWindow(nullptr), bValid(false) {
		gEngine = this;
	}

	Error Engine::startup(const std::string& configPath) {

		if (!glfwInit())
		{
			Error err(ErrorCode::FAIL_GLFW_INIT);
			err.mMessage = "GLFW failed to initialize!";
			err.mSource = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		// Load config
		ifstream f(configPath);
		f >> mConfig;

		auto glConfig = mConfig["opengl"];
		uint32_t majorVersion = DEFAULT_VERSION_MAJOR;
		uint32_t minorVersion = DEFAULT_VERSION_MINOR;
		glConfig["v_major"].get_to(majorVersion);
		glConfig["v_minor"].get_to(minorVersion);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
		glfwSetErrorCallback(error_callback);

		mWindow = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
		if (!mWindow)
		{
			Error err(ErrorCode::FAIL_GLFW_WINDOW_INIT);
			err.mMessage = "GLFW failed to create window!";
			err.mSource = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		glfwMakeContextCurrent(mWindow);
		gladLoadGL(); // Use GLAD to load necessary extensions
		glfwSwapInterval(1); // Set VSync on

		// Start building the engine graph
		auto v = mGraph.addNode(this);
		mHandle = mGraph.issueHandle(v);

		// Create content manager
		mContent = new ContentManager();
		mGraph.addNode<ContentManager>(mContent, mHandle);

		// Set valid
		bValid = true;

		return Error(ErrorCode::SUCCESS);
	}

	bool Engine::valid() const {
		return !glfwWindowShouldClose(mWindow) && bValid;
	}

	void Engine::update() {
		glfwPollEvents(); // Update window!
	}

	void Engine::render() {
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);
		glViewport(0, 0, width, height);

		glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(mWindow);
	}

	void Engine::shutdown() {

		mContent->unloadAll();
		delete mContent;

		glfwDestroyWindow(mWindow);

		glfwTerminate();
	}
	void Engine::exit()
	{
		bValid = false;
	}
}