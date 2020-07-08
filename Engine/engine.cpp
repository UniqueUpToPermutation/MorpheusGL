#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "content.hpp"
#include "forwardrenderer.hpp"

using namespace std;

namespace Morpheus {
	Engine* gEngine = nullptr;
	Engine* engine() { return gEngine; }

	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	void cursorPosHandler(GLFWwindow* win, double x, double y) {
		for (auto f : gEngine->mCursorPosCallbacks)
			(*f)(win, x, y);
	}

	void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers) {
		for (auto f : gEngine->mMouseButtonCallbacks)
			(*f)(win, button, action, modifiers);
	}

	void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods) {
		for (auto f : gEngine->mKeyCallbacks)
			(*f)(win, key, scancode, action, mods);
	}

	void charHandler(GLFWwindow* win, unsigned int codepoint) {
		for (auto f : gEngine->mCharCallbacks)
			(*f)(win, codepoint);
	}

	void dropHandler(GLFWwindow* win, int count, const char** filenames) {
		for (auto f : gEngine->mDropCallbacks)
			(*f)(win, count, filenames);
	}

	void scrollHandler(GLFWwindow* win, double x, double y) {
		for (auto f : gEngine->mScrollCallbacks)
			(*f)(win, x, y);
	}

	void framebufferSizeHandler(GLFWwindow* win, int width, int height) {
		for (auto f : gEngine->mFramebufferSizeCallbacks)
			(*f)(win, width, height);
	}

	void Engine::bindCursorPosEvent(const f_cursor_pos_t* f) {
		mCursorPosCallbacks.insert(f);
	}
	void Engine::bindMouseButtonEvent(const f_mouse_button_t* f) {
		mMouseButtonCallbacks.insert(f);
	}
	void Engine::bindKeyEvent(const f_key_t* f) {
		mKeyCallbacks.insert(f);
	}
	void Engine::bindCharEvent(const f_char_t* f) {
		mCharCallbacks.insert(f);
	}
	void Engine::bindDropEvent(const f_drop_t* f) {
		mDropCallbacks.insert(f);
	}
	void Engine::bindScrollEvent(const f_scroll_t* f) {
		mScrollCallbacks.insert(f);
	}
	void Engine::bindFramebufferSizeEvent(const f_framebuffer_size_t* f) {
		mFramebufferSizeCallbacks.insert(f);
	}

	void Engine::unbindCursorPosEvent(const f_cursor_pos_t* f) {
		mCursorPosCallbacks.erase(f);
	}
	void Engine::unbindMouseButtonEvent(const f_mouse_button_t* f) {
		mMouseButtonCallbacks.erase(f);
	}
	void Engine::unbindKeyEvent(const f_key_t* f) {
		mKeyCallbacks.erase(f);
	}
	void Engine::unbindCharEvent(const f_char_t* f) {
		mCharCallbacks.erase(f);
	}
	void Engine::unbindDropEvent(const f_drop_t* f) {
		mDropCallbacks.erase(f);
	}
	void Engine::unbindScrollEvent(const f_scroll_t* f) {
		mScrollCallbacks.erase(f);
	}
	void Engine::unbindFramebufferSizeEvent(const f_framebuffer_size_t* f) {
		mFramebufferSizeCallbacks.erase(f);
	}

	Engine::Engine() : mWindow(nullptr), bValid(false) {
		gEngine = this;
		NodeMetadata::init();
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
		f >> mConfig;

		// Create renderer
		auto renderer = new ForwardRenderer();
		v = mGraph.addNode(renderer, mHandle);
		renderer->mHandle = mGraph.issueHandle(v);
		mRenderer = renderer;

		// Renderer may request framebuffer features from GLFW
		mRenderer->postGlfwRequests();

		glfwSetErrorCallback(error_callback);

		mWindow = glfwCreateWindow(640, 480, "Morpheus Engine", NULL, NULL);
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
		v = mGraph.addNode(mContent, mHandle);
		mContent->mHandle = mGraph.issueHandle(v);
		
		// Add updater to the graph
		v = mGraph.addNode(&mUpdater, mHandle);
		mUpdater.mHandle = mGraph.issueHandle(v);

		// Initialize the renderer
		mRenderer->init(); 

		// Set valid
		bValid = true;

		// Set appropriate window callbacks
		glfwSetCursorPosCallback(mWindow, &cursorPosHandler);
		glfwSetMouseButtonCallback(mWindow, &mouseButtonHandler);
		glfwSetKeyCallback(mWindow, &keyHandler);
		glfwSetCharCallback(mWindow, &charHandler);
		glfwSetDropCallback(mWindow, &dropHandler);
		glfwSetScrollCallback(mWindow, &scrollHandler);
		glfwSetFramebufferSizeCallback(mWindow, &framebufferSizeHandler);

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

		// Clean up anything disposable
		for (auto nodeIt = mGraph.vertices(); nodeIt.valid(); nodeIt.next()) {
			auto desc = mGraph.desc(nodeIt());
			if (NodeMetadata::isDisposable(desc->type))
				dispose(desc->owner);
		}

		glfwDestroyWindow(mWindow);

		glfwTerminate();
	}
	void Engine::exit()
	{
		bValid = false;
	}
}