#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "content.hpp"
#include "forwardrenderer.hpp"
#include "input.hpp"

using namespace std;

namespace Morpheus {

	Engine* gEngine = nullptr;
	Engine* engine() { return gEngine; }

	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	Engine::Engine() : mWindow(nullptr), bValid(false) {
		gEngine = this;
		NodeMetadata::init();
	}

	DisplayParameters Engine::displayParams() const {
		DisplayParameters params;
		glfwGetFramebufferSize(mWindow, &params.mFramebufferWidth,
			&params.mFramebufferHeight);
		return params;
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

		// Start building the engine graph
		auto v = mGraph.addNode(this);
		mHandle = mGraph.issueHandle(v);

		// Load config
		ifstream f(configPath);
		if (!f.is_open()) {
			Error err(ErrorCode::FAIL_LOAD_CONFIG);
			err.mMessage = "Failed to load configuration file!";
			err.mSource = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}
		f >> mConfig;
		f.close();

		// Create renderer
		auto renderer = new ForwardRenderer();
		mGraph.addNode(renderer, mHandle);
		mRenderer = renderer;

		// Renderer may request framebuffer features from GLFW
		mRenderer->postGlfwRequests();

		glfwSetErrorCallback(error_callback);

		mWindow = glfwCreateWindow(800, 600, "Morpheus", NULL, NULL);
		if (!mWindow)
		{
			Error err(ErrorCode::FAIL_GLFW_WINDOW_INIT);
			err.mMessage = "GLFW failed to create window!";
			err.mSource = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		glfwMakeContextCurrent(mWindow);

		// Load OpenGL extensions
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			throw std::runtime_error("Could not initialize GLAD!");

		// Create content manager with handle
		mContent = new ContentManager();
		mGraph.addNode(mContent, mHandle);
		
		// Add updater to the graph
		mGraph.addNode(&mUpdater, mHandle);

		// Set appropriate window callbacks
		mInput.glfwRegister();

		// Initialize everything else
		init(node());

		// Set valid
		bValid = true;

		return Error(ErrorCode::SUCCESS);
	}

	bool Engine::valid() const {
		return !glfwWindowShouldClose(mWindow) && bValid;
	}

	void Engine::update() {
		glfwPollEvents(); // Update window!

		mUpdater.updateChildren(); // Update everything else
	}

	void Engine::shutdown() {

		mInput.glfwUnregster();

		// Clean up anything disposable
		for (auto nodeIt = mGraph.vertices(); nodeIt.valid(); nodeIt.next()) {
			auto desc = mGraph.desc(nodeIt());
			auto disposeInterface = getInterface<IDisposable>(*desc);
			if (disposeInterface)
				disposeInterface->dispose();
		}

		glfwDestroyWindow(mWindow);

		glfwTerminate();
	}
	void Engine::exit()
	{
		bValid = false;
	}
}