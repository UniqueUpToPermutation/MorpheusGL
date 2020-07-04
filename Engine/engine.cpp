#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include "engine.hpp"
#include "content.hpp"

using namespace std;

#define DEFAULT_VERSION_MAJOR 3
#define DEFAULT_VERSION_MINOR 3

namespace Morpheus {
	Engine* gEngine = nullptr;
	Engine& engine() { return *gEngine; }

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
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_STENCIL_BITS, 8);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

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

		// Clean up anything disposable
		for (auto nodeIt = mGraph.vertices(); nodeIt.valid(); nodeIt.next()) {
			auto desc = mGraph.desc(nodeIt());
			if (NodeMetadata::isDisposable(desc.type))
				dispose(desc.owner);
		}

		glfwDestroyWindow(mWindow);

		glfwTerminate();
	}
	void Engine::exit()
	{
		bValid = false;
	}
}