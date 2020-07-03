#pragma once

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"

#include <set>

/*
*	Every piece of content is treated as a node in the node graph.
*	A piece of content is owned by the content manager unless otherwise specified
*/

namespace Morpheus {
	
	class ContentManager;

	template <typename SubType> struct ContentSubTypeToBaseType {
	public:
		typedef void Base;
	};

	class IContentFactory : public IDisposable {
	private:
		NodeHandle mHandle;

	public:
		virtual ref<void> load(const std::string& source) = 0;
		virtual void unload(ref<void>& ref) = 0;
		inline NodeHandle handle() const { return mHandle; }
		Node node() const { return graph()[mHandle]; }

		friend class ContentManager;
	};

	template <typename ContentType>
	class ContentFactory;

	class ContentManager : public IDisposable {
	private:
		NodeHandle mHandle;
		std::set<IContentFactory*> mFactories;
		std::unordered_map<NodeType, IContentFactory*> mTypeToFactory;
		DigraphVertexLookupView<std::string> mSources;

	public:
		NodeHandle handle() const { return mHandle; }
		Node node() const { return graph()[mHandle]; }

		ContentManager();

		template <typename ContentType> void addFactory() {
			IContentFactory* factory = new ContentFactory<ContentType>();
			mFactories.insert(factory);
			mTypeToFactory[NODE_TYPE(ContentType)] = factory;
			auto v = graph().addNode<IContentFactory>(factory, mHandle);
			factory->mHandle = graph().issueHandle(v);
		}

		template <typename ContentType> void removeFactory() {
			auto type = NODE_TYPE(ContentType);
			auto factory = mTypeToFactory[type];
			mFactories.erase(factory);
			mTypeToFactory.erase(type);
			graph().recallHandle(factory->getHandle());
			delete factory;
		}

		void addContentNode(Node& content) {
		
		}

		template <typename ContentType>
		Node loadNode(const std::string& source, Node& parent) {
			Node v;
			if (mSources.tryFind(source, &v))
				return v;
			else {
				NodeType type = NODE_TYPE(ContentType);

				auto factory = mTypeToFactory[type];
				auto ref = factory->load(source);
				v = graph().addNode<CONTENT_BASE_TYPE(ContentType)>(ref, parent);
				mSources.set(v, source);
				return v;
			}
		}

		template <typename ContentType>
		ref<ContentType> loadRef(const std::string& source, Node& parent) {
			Node v;
			if (mSources.tryFind(source, &v))
				return graph().desc(v).owner.as<ContentType>();
			else {
				NodeType type = NODE_TYPE(ContentType);

				auto factory = mTypeToFactory[type];
				auto ref = factory->load(source);
				v = graph().addNode(ref, type, parent);
				mSources.set(v, source);
				return ref.as<ContentType>();
			}
		}

		template <typename ContentType>
		inline Node loadNode(const std::string& source, const NodeHandle parentHandle) {
			Node v = graph()[parentHandle];
			return loadNode<ContentType>(source, v);
		}

		template <typename ContentType>
		inline ref<ContentType> loadRef(const std::string& source, const NodeHandle parentHandle) {
			Node v = graph()[parentHandle];
			return loadRef<ContentType>(source, v);
		}

		template <typename ContentType> 
		inline Node loadNode(const std::string& source) {
			// Set self as the parent of the new object
			return loadNode<ContentType>(source, mHandle);
		}

		template <typename ContentType>
		inline ref<ContentType> loadRef(const std::string& source) {
			// Set self as the parent of the new object
			return loadRef<ContentType>(source, mHandle);
		}

		void unloadNode(Node& node) {
			auto desc = graph().desc(node);
			// Get factory from type
			auto factory = mTypeToFactory[desc.type];
			if (factory != nullptr)
				factory->unload(desc.owner);
		}

		void unloadAll() {
			for (auto it = graph()[mHandle].getOutgoingNeighbors(); it.valid(); it.next()) {
				Node v = it();
				unloadNode(v);
			}
		}

		void dispose() override {
			unloadAll();
		}
	};
}