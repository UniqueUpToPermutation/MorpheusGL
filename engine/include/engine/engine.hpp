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
#include <string>

namespace Morpheus {

	class Scene;
	class Texture;

	struct DisplayParameters {
		int32_t mFramebufferWidth;
		int32_t mFramebufferHeight;
	};

	
	/// The Morpheus graphics engine.
	
	class Engine : public INodeOwner {
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
		// Component responsible for updating logic entities, any child of the
		// updater will be updated through the IUpdateable interface
		Updater* mUpdater;
		// Component responsible for handling input
		Input mInput;
		// Whether or not the engine is still valid, i.e., not exitting.
		bool bValid;

	public:

		// The global JSON configuration of the engine.
		// returns: A reference to the global JSON configuration. 
		inline nlohmann::json& config() { return mConfig; }
	
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
		inline Updater* updater() { return mUpdater; }

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
		void render(INodeOwner* scene);
		// Swaps the back and front buffers.
		void present();
		// Creates a new scene as a child of the engine.
		Scene* makeScene();
	};
	
	// The global engine.
	// returns: A reference to the engine. 
	Engine* engine();
	
	// The global scene graph.
	// returns: A reference to the scene graph. 
	inline NodeGraph* globalGraph() {
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
	inline INodeOwner* find(const std::string& name) {
		return globalGraph()->find(name);
	}
	inline void setName(const Node& n, const std::string& name) {
		globalGraph()->setName(n, name);
	}
	inline void recallName(const std::string& name) {
		globalGraph()->recallName(name);
	}
	inline void setName(const INodeOwner* n, const std::string& name) {
		globalGraph()->setName(n, name);
	}
	inline bool tryFind(const std::string& name, INodeOwner** out) {
		return globalGraph()->tryFind(name, out);
	}

	inline void createNode(INodeOwner* owner) {
		globalGraph()->createNode(owner);
	}
	inline void createNode(INodeOwner* owner, INodeOwner* parent) {
		globalGraph()->createNode(owner, parent);
	}
	inline void createNode(INodeOwner* owner, const std::string& parentName) {
		globalGraph()->createNode(owner, parentName);
	}

	class BlitShaderView;

	struct RenderSettings {
		uint mMSAASamples;
		uint mAnisotropySamples;
	};

	class IRenderer : public INodeOwner {
	public:
		inline IRenderer() : INodeOwner(NodeType::RENDERER) {
		}

		// Set the settings of the renderer
		virtual void setRenderSettings(const RenderSettings& settings) = 0;
		virtual RenderSettings getRenderSettings() const = 0;

		// Posts all necessary requests to glfw when creating a context
		virtual void postGlfwRequests() = 0;
		// Draw the given scene
		virtual void draw(INodeOwner* node) = 0;
		// Get the type of this renderer
		virtual RendererType getRendererType() const = 0;
		// Set the clear color of this renderer
		virtual void setClearColorEx(float r, float g, float b) = 0;

		// Same as other blit function except you can override the shader used
		virtual void blitEx(Texture* texture,
			const glm::vec2& lower,
			const glm::vec2& upper,
			Shader* shader, BlitShaderView* shaderView) = 0;
		
		// Perform a blit of a texture to screen for debugging purposes
		// lower: the lower bounds of the blit rectangle (in pixels)
		// upper: the upper bounds of the blit rectangle (in pixels)
		void blit(Texture* texture, 
			const glm::vec2& lower,
			const glm::vec2& upper);

		void blit(Texture* texture,
			const glm::vec2& position);
		
		void blit(Texture* texture);

		void blit(Texture* texture,
			const glm::vec2& position, 
			Shader* shader,
			BlitShaderView* shaderView);

		void blit(Texture* texture,
			Shader* shader, 
			BlitShaderView* shaderView);

		void blit(Texture* texture,
			const glm::vec2& lower,
			const glm::vec2& upper,
			Shader* shader, BlitShaderView* shaderView);
			
		// Set the clear color of this renderer
		inline void setClearColor(const glm::vec3& color) {
			setClearColorEx(color.x, color.y, color.z);
		}

		inline void setClearColor(float r, float g, float b) {
			setClearColorEx(r, g, b);
		}
	};

	void getFramebufferSize(int* width, int* height);
}