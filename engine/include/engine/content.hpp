/*
*	Morpheus Graphics Engine
*	Author: Philip Etter
*
*	File: content.hpp
*	Description: Defines the ContentManager, which manages assets that are loaded
*	by the engine. Loading and unloading assets is done through content factories.
*	The ContentManager also does basic garbage collection for assets which are no
*	longer in use.
*/
#pragma once

#include <engine/core.hpp>
#include <engine/pool.hpp>
#include <engine/engine.hpp>
#include <engine/glslpreprocessor.hpp>

#include <set>
#include <iostream>

/*
*	Every piece of content is treated as a node in the node graph.
*	A piece of content is owned by the content manager unless otherwise specified.
*/

namespace Morpheus {

	template <typename SubType> struct ContentSubTypeToBaseType {
	public:
		typedef void Base;
	};

	template <typename ContentType>
	struct ContentExtParams;

	template <>
	struct ContentExtParams<Shader>;
	template <>
	struct ContentExtParams<Texture>;

	template <>
	struct ContentExtParams<Framebuffer>;

	// An interface that all content factories must inherit from.
	// Defines the interface for loading and unloading assets.
	class IContentFactory {
	public:
		virtual INodeOwner* load(const std::string& source, Node loadInto) = 0;
		virtual INodeOwner* loadEx(const std::string& source, Node loadInto, const void* extParams);

		virtual void unload(INodeOwner* ref) = 0;
		virtual std::string getContentTypeString() const = 0;

		virtual ~IContentFactory();

		friend class ContentManager;
	};

	template <typename ContentType>
	class ContentFactory;

	// Manages all loaded assets for the engine. Assets are loaded and unloaded from content
	// factories. All loaded assets are placed into the scene graph, and typically have the
	// content manager as one of their parents. The content manager can also do basic garbage
	// collection, whereby it checks for each of its children whether or not they have exactly
	// one parent - if they do, they are unloaded by their respective content manager and removed
	// from the scene graph. 
	class ContentManager : public INodeOwner {
	private:
		std::set<IContentFactory*> mFactories;
		std::unordered_map<NodeType, IContentFactory*> mTypeToFactory;
		DigraphTwoWayVertexLookupView<std::string> mSources;
		std::set<INodeOwner*> mMarkedNodes;

		ContentFactory<Texture>* mTextureFactory;
		ContentFactory<Shader>* mShaderFactory;
		ContentFactory<Sampler>* mSamplerFactory;
		ContentFactory<Material>* mMaterialFactory;
		ContentFactory<Framebuffer>* mFramebufferFactory;
		ContentFactory<Geometry>* mGeometryFactory;
		ContentFactory<HalfEdgeGeometry>* mHalfEdgeGeometryFactory;
		ContentFactory<StaticMesh>* mStaticMeshFactory;

	public:
		void init() override;
	
		ContentManager();
		~ContentManager() override;

		inline std::string getSourceString(INodeOwner* node) {
			std::string src;
			if (mSources.tryFind(node->node(), &src))
				return src;
			else
				return "UNNAMED";
		}

		// Add a factory to this content manager.
		// ContentType: The content type of the factory to add.
		template <typename ContentType> ContentFactory<ContentType>* addFactory() {
			auto factory = new ContentFactory<ContentType>();
			mFactories.insert(factory);
			mTypeToFactory[NODE_ENUM(ContentType)] = factory;
			return factory;
		}

		// Remove a factory from this content manager.
		// ContentType: The content type of the factory to remove.
		template <typename ContentType> void removeFactory() {
			auto type = NODE_ENUM(ContentType);
			auto factory = mTypeToFactory[type];
			mFactories.erase(factory);
			mTypeToFactory.erase(type);
			delete factory;
		}

		// Create a node for a content object and transfer ownership of node to content manager.
		// content: A reference to the content.
		// returns: A child node of the content manager with the content as an owner. 
		inline void createContentNode(INodeOwner* content) {
			graph()->createNode(content, this);
		}

		inline void createContentNode(INodeOwner* content, const std::string& source) {
			graph()->createNode(content, this);
			mSources.set(content->node(), source);
		}

		inline void createContentNode(INodeOwner* content, INodeOwner* parent) {
			createNode(content, this);
			parent->addChild(content);
		}

		inline void createContentNode(INodeOwner* content, INodeOwner* parent, const std::string& source) {
			createNode(content, this);
			parent->addChild(content);
			mSources.set(content->node(), source);
		}

		// Get a content factory by content type.
		// ContentType: The type of content to get a factory for
		// returns: The content factory. 
		template <typename ContentType> 
		ContentFactory<ContentType>* getFactory() {
			auto factory = mTypeToFactory[NODE_ENUM(ContentType)];
			return static_cast<ContentFactory<ContentType>*>(factory);
		}

		// Transfer ownership of an already existing node to the content manager.
		// Also adds the content to the source lookup so that it can be retrieved with
		// ContentManager::load.
		// content: The node for which to transfer ownership.
		// sourceName: The name of the content so it can be looked up with ContentManager::load.
		void setSource(INodeOwner* content, const std::string& sourceName) {
			mSources.set(content->node(), sourceName);
		}

		// Loads an asset for a parent node.
		// ContentType: Specifies the type of content. This determines which content factory is used.
		// source: The source (i.e., file path) to load from.
		// parent: Set a parent of the newly created node.
		// refOut: If this is not null, a ref to the loaded asset will be written to it.
		// returns: A node containing the asset. 
		template <typename ContentType>
		ContentType* load(const std::string& source, INodeOwner* parent = nullptr) {
			assert(IS_BASE_TYPE_<ContentType>::RESULT);

			std::string source_mod = source;
			std::replace(source_mod.begin(), source_mod.end(), '\\', '/');
			
			auto graph_ = graph();
			Node contentNode;
			INodeOwner* content;
			
			if (mSources.tryFind(source_mod, &contentNode)) {
				content = graph()->owner(contentNode);

				if (parent)
					parent->addChild(content);
					
				return convert<ContentType>(content);
			}
			else {
				// Create a vertex to load the content into
				contentNode = graph_->createVertex();
				
				// Load a ref via the correct content factory
				auto type = NODE_ENUM(ContentType);
				content = mTypeToFactory[type]->load(source_mod, contentNode);

				if (content == nullptr) {
					graph_->deleteVertex(contentNode);
					throw new std::runtime_error("Failed to load content!");
				}

				// Set the owner of the new vertex to the content just loaded
				graph_->setOwner(contentNode, content);

				// Set the node description of the created node appropriately
				mSources.set(contentNode, source_mod);
				
				// If a parent was passed, add the created content as a child of the parent
				if (parent)
					parent->addChild(content);

				// Add the new vertex as a child of the content manager
				addChild(content);

				return convert<ContentType>(content);
			}
		}

		template <typename ContentType>
		ContentType* loadEx(const std::string& source, const ContentExtParams<ContentType>& extParams, INodeOwner* parent = nullptr,
			bool bOverrideExistingSource = false) {
			assert(IS_BASE_TYPE_<ContentType>::RESULT);

			std::string source_mod = source;
			std::replace(source_mod.begin(), source_mod.end(), '\\', '/');
			
			auto graph_ = graph();
			Node contentNode;
			INodeOwner* content;

			bool bAlreadyExists = mSources.tryFind(source_mod, &contentNode);
			
			if (bAlreadyExists && !bOverrideExistingSource) {
				content = graph()->owner(contentNode);

				if (parent)
					parent->addChild(content);
					
				return convert<ContentType>(content);
			}
			else {
				if (bAlreadyExists) {
					std::cout << "Content " << source << " already exists! The content attached to this source will be overriden!" << std::endl;
				}

				// Create a vertex to load the content into
				contentNode = graph_->createVertex();
				
				// Load a ref via the correct content factory
				auto type = NODE_ENUM(ContentType);
				content = mTypeToFactory[type]->loadEx(source_mod, contentNode, &extParams);

				if (content == nullptr) {
					graph_->deleteVertex(contentNode);
					throw new std::runtime_error("Failed to load content!");
				}

				// Set the owner of the new vertex to the content just loaded
				graph_->setOwner(contentNode, content);

				// Set the node description of the created node appropriately
				mSources.set(contentNode, source_mod);
				
				// If a parent was passed, add the created content as a child of the parent
				if (parent)
					parent->addChild(content);

				// Add the new vertex as a child of the content manager
				addChild(content);

				return convert<ContentType>(content);
			}
		}

		// Perform garbage collection for all descendents that are no longer
		// in use.
		void collectGarbage();

		// Unload a node via its content factory and then remove it from the scene graph.
		// node: The node to unload
		void unload(INodeOwner* node);

		// Marks this node for an unload if necessary
		inline void markForUnload(INodeOwner* node) {
			mMarkedNodes.emplace(node);
		}

		// Checks to see if marked nodes are still in use, otherwise unloads them
		void unloadMarked();

		// Unload all children.
		void unloadAll();

		friend class Engine;
	};

	inline void createContentNode(INodeOwner* obj) {
		content()->createContentNode(obj);
	}

	inline void createContentNode(INodeOwner* obj, const std::string& source) {
		content()->createContentNode(obj, source);
	}

	inline void createContentNode(INodeOwner* obj, INodeOwner* parent) {
		content()->createContentNode(obj, parent);
	}

	inline void createContentNode(INodeOwner* obj, INodeOwner* parent, const std::string& source) {
		content()->createContentNode(obj, parent, source);
	}

	// Get a content factory by content type.
	// ContentType: The type of content to get a factory for
	// returns: The content factory. 
	template <typename ContentType> 
	inline ContentFactory<ContentType>* getFactory() {
		return content()->getFactory<ContentType>();
	}

	// Loads an asset
	template <typename ContentType> 
	inline ContentType* load(const std::string& source, INodeOwner* parent = nullptr) {
		return content()->load<ContentType>(source, parent);
	}

	// Loads an asset with extra parameters
	template <typename ContentType>
	ContentType* loadEx(const std::string& source, const ContentExtParams<ContentType>& extParams, INodeOwner* parent = nullptr,
		bool bOverrideExistingSource = false) {
		return content()->loadEx<ContentType>(source, extParams, parent, bOverrideExistingSource);
	}

	// Perform garbage collection for all descendents that are no longer
	// in use.
	inline void collectGarbage() {
		content()->collectGarbage();
	}

	// Unload a node via its content factory and then remove it from the scene graph.
	// node: The node to unload
	inline void unload(INodeOwner* node) {
		content()->unload(node);
	}

	// Unload all children.
	inline void unloadAll() {
		content()->unloadAll();
	}

	inline void unloadMarked() {
		content()->unloadMarked();
	}

	// Sets the source string of a content node.
	// node: The node which to set.
	// source: The content tag to set.
	inline void setSource(INodeOwner* node, const std::string& source) {
		content()->setSource(node, source);
	}

	inline void markForUnload(INodeOwner* node) {
		content()->markForUnload(node);
	}
}