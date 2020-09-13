/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: engine.hpp
*	Description: Defines the Morpheus graphics engine.
*/

#pragma once

#include <engine/core.hpp>
#include <engine/updater.hpp>
#include <engine/input.hpp>

#include <set>

namespace Morpheus {

	class Scene;
	class Texture;

	struct DisplayParameters {
		int32_t mFramebufferWidth;
		int32_t mFramebufferHeight;
	};

	
	/// The Morpheus graphics engine.
	
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

		// The global JSON configuration of the engine.
		// returns: A reference to the global JSON configuration. 
		inline nlohmann::json& config() { return mConfig; }
		
		// The scene graph of the engine.
		// returns: A reference to the scene graph. 
		inline NodeGraph* graph() { return &mGraph; }
		
		// The handle of the engine as a node in the scene graph.
		// returns: The handle to the engine. 
		inline NodeHandle handle() const { return mHandle; }
		
		// The node of the engine in the scene graph.
		// returns: The node of the engine in the scene graph 
		inline Node node() { return mGraph[mHandle]; }
		
		// The content manager of the engine.
		// returns: A reference to the content manager. 
		inline ContentManager* content() { return mContent; }
		
		// The engine's current GLFW window.
		// returns: A pointer to the engine's window. 
		inline GLFWwindow* window() { return mWindow; }
		
		// The engine's renderer.
		// returns: A pointer to the engine's renderer. 
		inline IRenderer* renderer() { return mRenderer; }
		
		// The engine's updater.
		// returns: A reference to the engine's updater. 
		inline Updater* updater() { return &mUpdater; }

		// The engine's input module.
		// returns: A reference to the engine's input module. 
		inline Input* input() { return &mInput; }
		
		// Gets the current display parameters.
		// returns: Display parameters
		DisplayParameters displayParams() const;

		Engine();

		// Initializes the engine and performs all necessary startup routines.
		// configPath: The path to the configuration file.
		// returns: Whether or not an error has occured.
		Error startup(const std::string& configPath);
		
		// Updates all necessary components of the engine.
		void update();
		
		// Release all memory in use and perform any necessary cleanup routines.
		void shutdown();
		
		// Force the engine to exit.
		void exit();
		
		// Gets whether or not the engine is still valid.	
		// returns: A flag determining if the engine is valid.
		bool valid() const;
		
		// Tells the renderer to render this scene.
		void render(Node scene);
		// Tells the renderer to render this scene.
		void render(NodeHandle sceneHandle);
		// Swaps the back and front buffers.
		void present();
		// Creates a new scene as a child of the engine.
		Node makeScene(ref<Scene>* sceneOut = nullptr);
	};
	
	// The global engine.
	// returns: A reference to the engine. 
	Engine* engine();
	
	// The global scene graph.
	// returns: A reference to the scene graph. 
	inline NodeGraph* graph() {
		return engine()->graph();
	}
	
	// The global content manager.
	// returns: A reference to the content manager. 
	inline ContentManager* content() {
		return engine()->content();
	}
	
	// The global renderer.
	// returns: A pointer to the renderer. 
	inline IRenderer* renderer() {
		return engine()->renderer();
	}
	
	// The global JSON configuration.
	// returns: A reference to the configuration of the engine. 
	inline nlohmann::json* config() {
		return &engine()->config();
	}
	
	// The GLFW window the engine is using.
	// returns: The GLFW window. 
	inline GLFWwindow* window() {
		return engine()->window();
	}

	// A reference to the global updater
	// returns: The global updater. 
	inline Updater* updater() {
		return engine()->updater();
	}
	// A reference to the global input module.
	// returns: The global input module. 
	inline Input* input() {
		return engine()->input();
	}

	// Return the description of a node.
	// n: The node.
	// returns: The description. 
	inline NodeData* desc(const Node& n) {
		return graph()->desc(n);
	}
	inline NodeData* desc(const std::string& name) {
		return desc((*graph())[name]);
	}
	inline NodeHandle issueHandle(const Node& n) {
		return graph()->issueHandle(n);
	}
	inline void setName(const Node& n, const std::string& name) {
		graph()->setName(n, name);
	}
	inline void recallName(const std::string& name) {
		graph()->recallName(name);
	}
	inline Node find(const NodeHandle h) {
		return graph()->find(h);
	}
	inline Node find(const std::string& name) {
		return graph()->find(name);
	}
	inline bool tryFind(const std::string& name, Node* out) {
		return graph()->tryFind(name, out);
	}
	inline void recallHandle(const NodeHandle handle) {
		graph()->recallHandle(handle);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, NodeType type) {
		return graph()->addNode<OwnerType>(owner, type);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, NodeType type, Node& parent) {
		return graph()->addNode<OwnerType>(owner, type, parent);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, NodeType type, NodeHandle parentHandle) {
		return graph()->addNode<OwnerType>(owner, type, parentHandle);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, NodeType type, const std::string& parentName) {
		return graph()->addNode<OwnerType>(owner, type, parentName);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, Node& parent) {
		return graph()->addNode<OwnerType>(owner, parent);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, NodeHandle parentHandle) {
		return graph()->addNode<OwnerType>(owner, parentHandle);
	}
	template <typename OwnerType>
	inline Node addNode(OwnerType* owner, const std::string& parentName) {
		return graph()->addNode<OwnerType>(owner, parentName);
	}
	inline Node addNode(ref<void> owner, NodeType type) {
		return graph()->addNode(owner, type);
	}
	inline Node addNode(ref<void> owner, NodeType type, Node& parent) {
		return graph()->addNode(owner, type, parent);
	}
	inline Node addNode(ref<void> owner, NodeType type, NodeHandle parentHandle) {
		return graph()->addNode(owner, type, parentHandle);
	}
	inline Node addNode(ref<void> owner, NodeType type, const std::string& parentName) {
		return graph()->addNode(owner, type, parentName);
	}
	template <typename OwnerType>
	inline Node addNode(ref<void> owner, Node& parent) {
		return graph()->addNode<OwnerType>(owner, parent);
	}
	template <typename OwnerType>
	inline Node addNode(ref<void> owner, NodeHandle parentHandle) {
		return graph()->addNode<OwnerType>(owner, parentHandle);
	}
	template <typename OwnerType>
	inline Node addNode(ref<void> owner, const std::string& parentName) {
		return graph()->addNode<OwnerType>(owner, parentName);
	}
	inline Node makeProxy(const Node& base) {
		return graph()->makeProxy(base);
	}
	inline Node makeContentProxy(const Node& base) {
		return graph()->makeContentProxy(base);
	}
	template <typename T> 
	inline Node addNode(T* owner) {
		return graph()->addNode<T>(owner);
	}
	template <typename T> 
	inline Node addNode(const ref<T>& owner) {
		return graph()->addNode<T>(owner);
	}

	class IRenderer : public IDisposable, public IInitializable {
	public:
		// Posts all necessary requests to glfw when creating a context
		virtual void postGlfwRequests() = 0;
		// Draw the given scene
		virtual void draw(Node scene) = 0;
		// Get the handle of this renderer
		virtual NodeHandle handle() const = 0;
		// Get the type of this renderer
		virtual RendererType getType() const = 0;
		// Set the clear color of this renderer
		virtual void setClearColor(float r, float g, float b) = 0;

		// Perform a blit of a texture to screen for debugging purposes
		// lower: the lower bounds of the blit rectangle (in pixels)
		// upper: the upper bounds of the blit rectangle (in pixels)
		virtual void debugBlit(ref<Texture> texture, 
			const glm::vec2& lower,
			const glm::vec2& upper) = 0;

		// Get the node of this renderer
		inline Node node() const { return (*graph())[handle()]; }
		// Set the clear color of this renderer
		void setClearColor(const glm::vec3& color) {
			setClearColor(color.x, color.y, color.z);
		}

		inline void draw(NodeHandle scene) {
			draw(graph()->find(scene));
		}
	};
}