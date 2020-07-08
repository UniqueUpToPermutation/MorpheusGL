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

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"

#include <set>

/*
*	Every piece of content is treated as a node in the node graph.
*	A piece of content is owned by the content manager unless otherwise specified.
*/

namespace Morpheus {
	
	class ContentManager;

	template <typename SubType> struct ContentSubTypeToBaseType {
	public:
		typedef void Base;
	};

	/// <summary>
	/// An interface that all content factories must inherit from.
	/// Defines the interface for loading and unloading assets.
	/// </summary>
	class IContentFactory : public IDisposable {
	public:
		virtual ref<void> load(const std::string& source, Node& loadInto) = 0;
		virtual void unload(ref<void>& ref) = 0;

		friend class ContentManager;
	};

	template <typename ContentType>
	class ContentFactory;

	/// <summary>
	/// Manages all loaded assets for the engine. Assets are loaded and unloaded from content
	/// factories. All loaded assets are placed into the scene graph, and typically have the
	/// content manager as one of their parents. The content manager can also do basic garbage
	/// collection, whereby it checks for each of its children whether or not they have exactly
	/// one parent - if they do, they are unloaded by their respective content manager and removed
	/// from the scene graph. 
	/// </summary>
	class ContentManager : public IDisposable {
	private:
		NodeHandle mHandle;
		std::set<IContentFactory*> mFactories;
		std::unordered_map<NodeType, IContentFactory*> mTypeToFactory;
		DigraphVertexLookupView<std::string> mSources;

	public:
		/// <summary>
		/// The handle of this content manager in the scene graph.
		/// </summary>
		/// <returns>The handle of this content manager.</returns>
		NodeHandle handle() const { return mHandle; }
		/// <summary>
		/// The node of this content manager in the scene graph.
		/// Do not store this value as it may change if the graph's
		/// memory is altered.
		/// </summary>
		/// <returns>The node of this content manager.</returns>
		Node node() const { return (*graph())[mHandle]; }

		ContentManager();

		/// <summary>
		/// Add a factory to this content manager.
		/// </summary>
		/// <typeparam name="ContentType">The content type of the factory to add.</typeparam>
		template <typename ContentType> void addFactory() {
			IContentFactory* factory = new ContentFactory<ContentType>();
			mFactories.insert(factory);
			mTypeToFactory[NODE_TYPE(ContentType)] = factory;
		}

		/// <summary>
		/// Remove a factory from this content manager.
		/// </summary>
		/// <typeparam name="ContentType">The content type of the factory to remove.</typeparam>
		template <typename ContentType> void removeFactory() {
			auto type = NODE_TYPE(ContentType);
			auto factory = mTypeToFactory[type];
			mFactories.erase(factory);
			mTypeToFactory.erase(type);
			delete factory;
		}

		/// <summary>
		/// Transfer ownership of an already existing node to the content manager.
		/// </summary>
		/// <param name="content">The node for which to transfer ownership.</param>
		void addContentNode(Node& content) {
			auto self = node();
			graph()->createEdge(self, content);
		}
		

		/// <summary>
		/// Transfer ownership of an already existing node to the content manager.
		/// Also adds the content to the source lookup so that it can be retrieved with
		/// ContentManager::load.
		/// </summary>
		/// <param name="content">The node for which to transfer ownership.</param>
		/// <param name="sourceName">The name of the content so it can be looked up with ContentManager::load.</param>
		void addContentNode(Node& content, std::string& sourceName) {
			auto self = node();
			graph()->createEdge(self, content);
			mSources.set(content, sourceName);
		}

		/// <summary>
		/// Loads an asset for a parent node.
		/// </summary>
		/// <typeparam name="ContentType">Specifies the type of content. This determines which content factory is used.</typeparam>
		/// <param name="source">The source (i.e., file path) to load from.</param>
		/// <param name="parent">Set a parent of the newly created node.</param>
		/// <param name="refOut">If this is not null, a ref to the loaded asset will be written to it.</param>
		/// <returns>A node containing the asset.</returns>
		template <typename ContentType>
		Node load(const std::string& source, const Node& parent, ref<ContentType>* refOut = nullptr) {
			std::string source_mod = source;
			std::replace(source_mod.begin(), source_mod.end(), '\\', '/');
			
			auto graph_ = graph();
			Node v;
			if (mSources.tryFind(source_mod, &v)) {
				assert(graph_->desc(v)->type == NODE_TYPE(ContentType));
				*refOut = graph_->desc(v)->owner.as<ContentType>();
				return v;
			}
			else {
				// Create a vertex to load the content into
				v = graph_->createVertex();
				// Add the new vertex as a child of the content manager
				graph_->createEdge(node(), v);
				
				// Load a ref via the correct content factory
				auto type = NODE_TYPE(ContentType);
				auto ref = mTypeToFactory[type]->load(source_mod, v);

				// Set the node description of the created node appropriately
				auto desc = graph_->desc(v);
				desc->type = type;
				desc->owner = ref;
				
				// If a parent was passed, add the created content as a child of the parent
				if (parent.isValid())
					graph_->createEdge(parent, v);

				mSources.set(v, source_mod);

				// Return the ref for convienience.
				if (refOut)
					*refOut = ref.as<ContentType>();

				return v;
			}
		}

		/// <summary>
		/// Loads an asset for a parent node.
		/// </summary>
		/// <typeparam name="ContentType">Specifies the type of content. This determines which content factory is used.</typeparam>
		/// <param name="source">The source (i.e., file path) to load from.</param>
		/// <param name="parentHandle">Set a parent of the newly created node.</param>
		/// <param name="refOut">If this is not null, a ref to the loaded asset will be written to it.</param>
		/// <returns>A node containing the asset.</returns>
		template <typename ContentType>
		inline Node load(const std::string& source, const NodeHandle parentHandle, ref<ContentType>* refOut = nullptr) {
			Node v = graph()[parentHandle];
			return load<ContentType>(source, v, refOut);
		}

		/// <summary>
		/// Loads an asset.
		/// </summary>
		/// <typeparam name="ContentType">Specifies the type of content. This determines which content factory is used.</typeparam>
		/// <param name="source">The source (i.e., file path) to load from.</param>
		/// <param name="refOut">If this is not null, a ref to the loaded asset will be written to it.</param>
		/// <returns>A node containing the asset.</returns>
		template <typename ContentType> 
		inline Node load(const std::string& source, ref<ContentType>* refOut = nullptr) {
			// Set self as the parent of the new object
			return load<ContentType>(source, Node::invalid(), refOut);
		}

		/// <summary>
		/// Perform garbage collection for all descendents that are no longer
		/// in use.
		/// </summary>
		void collectGarbage();

		/// <summary>
		/// Unload a node via its content factory and then remove it from the scene graph.
		/// </summary>
		/// <param name="node">The node to unload</param>
		void unload(Node& node);

		/// <summary>
		/// Unload all children.
		/// </summary>
		void unloadAll();

		/// <summary>
		/// Dispose of the content manager
		/// </summary>
		void dispose() override;

		friend class Engine;
	};
}