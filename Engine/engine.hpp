/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: engine.hpp
*	Description: Defines the Morpheus graphics engine.
*/

#pragma once

#include "core.hpp"
#include "updater.hpp"
#include "input.hpp"

#include <set>

namespace Morpheus {

	struct DisplayParameters {
		int32_t mFramebufferWidth;
		int32_t mFramebufferHeight;
	};

	/// <summary>
	/// The Morpheus graphics engine.
	/// </summary>
	class Engine  {
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
		// Component responsible for updating logic entities, any child of the
		// updater will be updated through the IUpdateable interface
		Updater mUpdater;
		// Component responsible for handling input
		Input mInput;
		// Whether or not the engine is still valid, i.e., not exitting.
		bool bValid;

	public:

		/// <summary>
		/// The global JSON configuration of the engine.
		/// </summary>
		/// <returns>A reference to the global JSON configuration.</returns>
		inline nlohmann::json& config() { return mConfig; }
		/// <summary>
		/// The scene graph of the engine.
		/// </summary>
		/// <returns>A reference to the scene graph.</returns>
		inline NodeGraph* graph() { return &mGraph; }
		/// <summary>
		/// The handle of the engine as a node in the scene graph.
		/// </summary>
		/// <returns>The handle to the engine.</returns>
		inline NodeHandle handle() const { return mHandle; }
		/// <summary>
		/// The node of the engine in the scene graph.
		/// </summary>
		/// <returns>The node of the engine in the scene graph</returns>
		inline Node node() { return mGraph[mHandle]; }
		/// <summary>
		/// The content manager of the engine.
		/// </summary>
		/// <returns>A reference to the content manager.</returns>
		inline ContentManager* content() { return mContent; }
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
		/// <summary>
		/// The engine's updater.
		/// </summary>
		/// <returns>A reference to the engine's updater.</returns>
		inline Updater* updater() { return &mUpdater; }
		/// <summary>
		/// The engine's input module.
		/// </summary>
		/// <returns>A reference to the engine's input module.</returns>
		inline Input* input() { return &mInput; }
		/// <summary>
		/// Gets the current display parameters.
		/// </summary>
		/// <returns>Display parameters</returns>
		DisplayParameters displayParams() const;

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
	};

	/// <summary>
	/// The global engine.
	/// </summary>
	/// <returns>A reference to the engine.</returns>
	Engine* engine();
	/// <summary>
	/// The global scene graph.
	/// </summary>
	/// <returns>A reference to the scene graph.</returns>
	inline NodeGraph* graph() {
		return engine()->graph();
	}
	/// <summary>
	/// The global content manager.
	/// </summary>
	/// <returns>A reference to the content manager.</returns>
	inline ContentManager* content() {
		return engine()->content();
	}
	/// <summary>
	/// The global renderer.
	/// </summary>
	/// <returns>A pointer to the renderer.</returns>
	inline IRenderer* renderer() {
		return engine()->renderer();
	}
	/// <summary>
	/// The global JSON configuration.
	/// </summary>
	/// <returns>A reference to the configuration of the engine.</returns>
	inline nlohmann::json* config() {
		return &engine()->config();
	}
	/// <summary>
	/// The GLFW window the engine is using.
	/// </summary>
	/// <returns>The GLFW window.</returns>
	inline GLFWwindow* window() {
		return engine()->window();
	}
	/// <summary>
	/// A reference to the global updater
	/// </summary>
	/// <returns>The global updater.</returns>
	inline Updater* updater() {
		return engine()->updater();
	}

	/// <summary>
	/// A reference to the global input module.
	/// </summary>
	/// <returns>The global input module.</returns>
	inline Input* input() {
		return engine()->input();
	}

	/// <summary>
	/// Return the description of a node.
	/// </summary>
	/// <param name="n">The node.</param>
	/// <returns>The description.</returns>
	inline NodeData* desc(const Node& n) {
		return graph()->desc(n);
	}

	inline NodeData* desc(const std::string& name) {
		return desc((*graph())[name]);
	}

	class IRenderer : public IDisposable, public IInitializable {
	public:
		virtual void postGlfwRequests() = 0;
		virtual void draw(Node& scene) = 0;
		virtual NodeHandle handle() const = 0;
		virtual RendererType getType() const = 0;
		inline Node node() const { return (*graph())[handle()]; }
	};
}