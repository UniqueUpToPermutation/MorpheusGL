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

#include <set>

/*
*	Every piece of content is treated as a node in the node graph.
*	A piece of content is owned by the content manager unless otherwise specified.
*/

namespace Morpheus {

	template <typename SubType> struct ContentSubTypeToBaseType {
	public:
		typedef void Base;
	};

	// An interface that all content factories must inherit from.
	// Defines the interface for loading and unloading assets.
	class IContentFactory {
	public:
		virtual ref<void> load(const std::string& source, Node& loadInto) = 0;
		virtual void unload(ref<void>& ref) = 0;
		virtual void dispose() = 0;

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
	class ContentManager : public IDisposable, public IInitializable {
	private:
		NodeHandle mHandle;
		std::set<IContentFactory*> mFactories;
		std::unordered_map<NodeType, IContentFactory*> mTypeToFactory;
		DigraphTwoWayVertexLookupView<std::string> mSources;

	public:
		void init(Node node) override;

		// The handle of this content manager in the scene graph.
		// returns: The handle of this content manager. 
		NodeHandle handle() const { return mHandle; }
		
		// The node of this content manager in the scene graph.
		// Do not store this value as it may change if the graph's
		// memory is altered.
		// returns: The node of this content manager. 
		Node node() const { return (*graph())[mHandle]; }

		ContentManager();

		// Add a factory to this content manager.
		// ContentType: The content type of the factory to add.
		template <typename ContentType> void addFactory() {
			IContentFactory* factory = new ContentFactory<ContentType>();
			mFactories.insert(factory);
			mTypeToFactory[NODE_ENUM(ContentType)] = factory;
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

		// Transfer ownership of an already existing node to the content manager.
		// content: The node for which to transfer ownership.
		void addContentNode(Node& content) {
			auto self = node();
			graph()->createEdge(self, content);
		}

		// Create a node for a content object and transfer ownership of node to content manager.
		// ContentType: The type of the content.
		// content: A reference to the content.
		// returns: A child node of the content manager with the content as an owner. 
		template <typename ContentType>
		Node createContentNode(ref<ContentType>& content) {
			auto newNode = graph()->addNode(content);
			addContentNode(newNode);
			return newNode;
		}

		// Create a node for a content object and transfer ownership of node to content manager.
		// ContentType: The type of the content.
		// content: A reference to the content.
		// parent: The parent of this node (i.e., scene, user, etc.), so that it does not get
		// deallocated upon garbage collection
		// returns: A child node of the content manager with the content as an owner. 
		template <typename ContentType>
		Node createContentNode(ref<ContentType>& content, Node parent) {
			auto newNode = graph()->addNode(content);
			addContentNode(newNode);
			if (parent.valid())
				graph()->createEdge(parent, newNode);
			return newNode;
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
		void addContentNode(Node& content, const std::string& sourceName) {
			auto self = node();
			graph()->createEdge(self, content);
			mSources.set(content, sourceName);
		}

		// Loads an asset for a parent node.
		// ContentType: Specifies the type of content. This determines which content factory is used.
		// source: The source (i.e., file path) to load from.
		// parent: Set a parent of the newly created node.
		// refOut: If this is not null, a ref to the loaded asset will be written to it.
		// returns: A node containing the asset. 
		template <typename ContentType>
		Node load(const std::string& source, const Node& parent, ref<ContentType>* refOut = nullptr) {
			assert(IS_BASE_TYPE_<ContentType>::RESULT);

			std::string source_mod = source;
			std::replace(source_mod.begin(), source_mod.end(), '\\', '/');
			
			auto graph_ = graph();
			Node v;
			if (mSources.tryFind(source_mod, &v)) {
				assert(graph_->desc(v)->type == NODE_ENUM(ContentType));

				// Return the ref for convienience.
				if (refOut)
					*refOut = graph_->desc(v)->owner.reinterpret<ContentType>();

				return v;
			}
			else {
				// Create a vertex to load the content into
				v = graph_->createVertex();
				// Add the new vertex as a child of the content manager
				graph_->createEdge(node(), v);
				
				// Load a ref via the correct content factory
				auto type = NODE_ENUM(ContentType);
				ref<void> obj_ref = mTypeToFactory[type]->load(source_mod, v);

				// Set the node description of the created node appropriately
				auto desc = graph_->desc(v);
				desc->type = type;
				desc->owner = obj_ref;
				
				// If a parent was passed, add the created content as a child of the parent
				if (parent.valid())
					graph_->createEdge(parent, v);

				mSources.set(v, source_mod);

				// Return the ref for convienience.
				if (refOut)
					*refOut = obj_ref.reinterpret<ContentType>();

				return v;
			}
		}

		// Loads an asset for a parent node.
		// ContentType: Specifies the type of content. This determines which content factory is used.
		// source: The source (i.e., file path) to load from.
		// parentHandle: Set a parent of the newly created node.
		// refOut: If this is not null, a ref to the loaded asset will be written to it.
		// returns: A node containing the asset. 
		template <typename ContentType>
		inline Node load(const std::string& source, const NodeHandle parentHandle, ref<ContentType>* refOut = nullptr) {
			Node v = graph()->find(parentHandle);
			return load<ContentType>(source, v, refOut);
		}

		// Loads an asset.
		// ContentType: Specifies the type of content. This determines which content factory is used.
		// source: The source (i.e., file path) to load from.
		// refOut: If this is not null, a ref to the loaded asset will be written to it.
		// returns: A node containing the asset. 
		template <typename ContentType> 
		inline Node load(const std::string& source, ref<ContentType>* refOut = nullptr) {
			// Set self as the parent of the new object
			return load<ContentType>(source, Node::invalid(), refOut);
		}

		// Perform garbage collection for all descendents that are no longer
		// in use.
		void collectGarbage();

		// Unload a node via its content factory and then remove it from the scene graph.
		// node: The node to unload
		void unload(Node& node);

		// Unload all children.
		void unloadAll();

		// Sets the source string of a content node.
		// node: The node which to set.
		// source: The content tag to set.
		void setSource(const Node& node, const std::string& source);

		// Dispose of the content manager
		void dispose() override;

		friend class Engine;
	};
}