/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: engine.hpp
*	Description: Defines the Morpheus graphics engine.
*/

#pragma once

#include "core.hpp"

#include <set>

namespace Morpheus {

	typedef std::function<void(GLFWwindow*, double, double)> f_cursor_pos_t;
	typedef std::function<void(GLFWwindow*, int, int, int)> f_mouse_button_t;
	typedef std::function<void(GLFWwindow*, int, int, int, int)> f_key_t;
	typedef std::function<void(GLFWwindow*, unsigned int)> f_char_t;
	typedef std::function<void(GLFWwindow*, int, const char**)> f_drop_t;
	typedef std::function<void(GLFWwindow*, double, double)> f_scroll_t;
	typedef std::function<void(GLFWwindow*, int, int)> f_framebuffer_size_t;

	/// <summary>
	/// The Morpheus graphics engine.
	/// </summary>
	class Engine {
	private:
		// The window the engine is rendering to
		GLFWwindow* mWindow;
		// The global JSON configuration for the engine
		nlohmann::json mConfig;
		// The scene graph
		NodeGraph mGraph;
		// The content manager
		ContentManager* mContent;
		// The renderer
		IRenderer* mRenderer;
		// The handle for the engine node in the scene graph
		NodeHandle mHandle;
		// Whether or not the engine is still valid, i.e., not exitting.
		bool bValid;

		std::set<const f_cursor_pos_t*> mCursorPosCallbacks;
		std::set<const f_mouse_button_t*> mMouseButtonCallbacks;
		std::set<const f_key_t*> mKeyCallbacks;
		std::set<const f_char_t*> mCharCallbacks;
		std::set<const f_drop_t*> mDropCallbacks;
		std::set<const f_scroll_t*> mScrollCallbacks;
		std::set<const f_framebuffer_size_t*> mFramebufferSizeCallbacks;

	public:

		// Events that can be bound to
		void bindCursorPosEvent(const f_cursor_pos_t* f);
		void bindMouseButtonEvent(const f_mouse_button_t* f);
		void bindKeyEvent(const f_key_t* f);
		void bindCharEvent(const f_char_t* f);
		void bindDropEvent(const f_drop_t* f);
		void bindScrollEvent(const f_scroll_t* f);
		void bindFramebufferSizeEvent(const f_framebuffer_size_t* f);

		void unbindCursorPosEvent(const f_cursor_pos_t* f);
		void unbindMouseButtonEvent(const f_mouse_button_t* f);
		void unbindKeyEvent(const f_key_t* f);
		void unbindCharEvent(const f_char_t* f);
		void unbindDropEvent(const f_drop_t* f);
		void unbindScrollEvent(const f_scroll_t* f);
		void unbindFramebufferSizeEvent(const f_framebuffer_size_t* f);

		/// <summary>
		/// The global JSON configuration of the engine.
		/// </summary>
		/// <returns>A reference to the global JSON configuration.</returns>
		inline nlohmann::json& config() { return mConfig; }
		/// <summary>
		/// The scene graph of the engine.
		/// </summary>
		/// <returns>A reference to the scene graph.</returns>
		inline NodeGraph& graph() { return mGraph; }
		/// <summary>
		/// The handle of the engine as a node in the scene graph.
		/// </summary>
		/// <returns>The handle to the engine.</returns>
		inline NodeHandle handle() const { return mHandle; }
		/// <summary>
		/// The content manager of the engine.
		/// </summary>
		/// <returns>A reference to the content manager.</returns>
		inline ContentManager& content() { return *mContent; }
		/// <summary>
		/// The engine's current GLFW window.
		/// </summary>
		/// <returns>A pointer to the engine's window.</returns>
		inline GLFWwindow* window() { return mWindow; }
		/// <summary>
		/// The engine's renderer.
		/// </summary>
		/// <returns>A pointer to the engine's renderer.</returns>
		inline IRenderer* renderer() { return mRenderer; }

		Engine();

		/// <summary>
		/// Initializes the engine and performs all necessary startup routines.
		/// </summary>
		/// <param name="configPath">The path to the configuration file.</param>
		/// <returns>Whether or not an error has occured.</returns>
		Error startup(const std::string& configPath);
		/// <summary>
		/// Updates all necessary components of the engine.
		/// </summary>
		void update();
		/// <summary>
		/// Perform a render.
		/// </summary>
		void render();
		/// <summary>
		/// Release all memory in use and perform any necessary cleanup routines.
		/// </summary>
		void shutdown();
		/// <summary>
		/// Force the engine to exit.
		/// </summary>
		void exit();
		/// <summary>
		/// Gets whether or not the engine is still valid.
		/// </summary>
		/// <returns>A flag determining if the engine is valid.</returns>
		bool valid() const;

		friend void cursorPosHandler(GLFWwindow* win, double x, double y);
		friend void mouseButtonHandler(GLFWwindow* win, int button, int action, int modifiers);
		friend void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mods);
		friend void charHandler(GLFWwindow* win, unsigned int codepoint);
		friend void dropHandler(GLFWwindow* win, int count, const char** filenames);
		friend void scrollHandler(GLFWwindow* win, double x, double y);
		friend void framebufferSizeHandler(GLFWwindow* win, int width, int height);
	};

	/// <summary>
	/// The global engine.
	/// </summary>
	/// <returns>A reference to the engine.</returns>
	Engine& engine();
	/// <summary>
	/// The global scene graph.
	/// </summary>
	/// <returns>A reference to the scene graph.</returns>
	inline NodeGraph& graph() {
		return engine().graph();
	}
	/// <summary>
	/// The global content manager.
	/// </summary>
	/// <returns>A reference to the content manager.</returns>
	inline ContentManager& content() {
		return engine().content();
	}
	/// <summary>
	/// The global renderer.
	/// </summary>
	/// <returns>A pointer to the renderer.</returns>
	inline IRenderer* renderer() {
		return engine().renderer();
	}
	/// <summary>
	/// The global JSON configuration.
	/// </summary>
	/// <returns>A reference to the configuration of the engine.</returns>
	inline nlohmann::json& config() {
		return engine().config();
	}
}