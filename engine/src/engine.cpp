#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <engine/engine.hpp>
#include <engine/content.hpp>
#include <engine/forwardrenderer.hpp>
#include <engine/input.hpp>
#include <engine/scene.hpp>
#include <engine/camera.hpp>

using namespace std;

namespace Morpheus {

	Engine* gEngine = nullptr;
	Engine* engine() { return gEngine; }

	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	Engine::Engine() : INodeOwner(NodeType::ENGINE), mWindow(nullptr), bValid(false) {
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

		glfwSetErrorCallback(error_callback);

		if (!glfwInit())
		{
			Error err(ErrorCode::FAIL_GLFW_INIT);
			err.mMessage = "GLFW failed to initialize!";
			err.mSource = "Engine::startup";
			cout << err.str() << endl;
			return err;
		}

		// Start building the engine graph
		mGraph.createNode(this);

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
		mGraph.createNode(renderer, this);
		mRenderer = renderer;

		// Renderer may request framebuffer features from GLFW
		mRenderer->postGlfwRequests();

		int width = 800;
		int height = 600;
		std::string title = "Morpheus";
		if (mConfig.contains("window")) {
			auto& windowConfig = mConfig["window"];
			if (windowConfig.contains("width")) windowConfig["width"].get_to(width);
			if (windowConfig.contains("height")) windowConfig["height"].get_to(height);
			if (windowConfig.contains("title")) windowConfig["title"].get_to(title);
		}

		mWindow = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
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
		mGraph.createNode(mContent, this);
		
		// Add updater to the graph
		mUpdater = new Updater();
		mGraph.createNode(mUpdater, this);

		// Set appropriate window callbacks
		mInput.glfwRegister();

		// Initialize everything else
		::Morpheus::init(this);

		// Set valid
		bValid = true;

		return Error(ErrorCode::SUCCESS);
	}

	bool Engine::valid() const {
		return !glfwWindowShouldClose(mWindow) && bValid;
	}

	void Engine::render(INodeOwner* scene) {
		mRenderer->draw(scene);
	}

	void Engine::present() {
		glfwSwapBuffers(mWindow);
	}

	Scene* Engine::makeScene() {
		auto scene = new Scene();
		mGraph.createNode(scene, this);
		return scene;
	}

	void Engine::update() {
		glfwPollEvents(); // Update window!

		mUpdater->updateChildren(); // Update everything else
	}

	void Engine::shutdown() {

		mInput.glfwUnregster();

		// Clean up anything disposable
		for (auto it = children(); it.valid();) {
			auto child = it();
			it.next();
			if (child != mContent)
				prune(child);
		}
		
		// Explicitly dispose the content manager last to clean up anything remaining
		delete mContent;
		mContent = nullptr;

		glfwDestroyWindow(mWindow);

		glfwTerminate();
	}
	
	void Engine::exit()
	{
		bValid = false;
	}

	void IRenderer::debugBlit(Texture* texture,
		const glm::vec2& position) {
		debugBlit(texture, position, position + glm::vec2(texture->width(), texture->height()));
	}

	void IRenderer::debugBlit(Texture* texture) {
		debugBlit(texture, glm::vec2(0.0, 0.0), glm::vec2(texture->width(), texture->height()));
	}
}