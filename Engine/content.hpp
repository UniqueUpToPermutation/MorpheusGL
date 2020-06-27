#pragma once

#include "core.hpp"
#include "pool.hpp"
#include "engine.hpp"

#include <set>

#define REGISTER_CONTENT_BASE_TYPE(subType, baseType) template <> struct ContentSubTypeToBaseType<subType> { \
	public: \
	typedef baseType Base; \
	}; \

#define CONTENT_BASE_TYPE(subType) ContentSubTypeToBaseType<subType>::Base

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

		friend class ContentManager;
	};
	REGISTER_CONTENT_BASE_TYPE(IContentFactory, IContentFactory);

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

		template <typename ContentType>
		DigraphVertex loadNode(const std::string& source, DigraphVertex& parent) {
			DigraphVertex v;
			if (mSources.tryFind(source, &v))
				return v;
			else {
				NodeType type = NODE_TYPE(CONTENT_BASE_TYPE(ContentType));

				auto factory = mTypeToFactory[type];
				auto ref = factory->load(source);
				v = graph().addNode<CONTENT_BASE_TYPE(ContentType)>(ref, parent);
				mSources.set(v, source);
				return v;
			}
		}

		template <typename ContentType>
		ref<ContentType> loadRef(const std::string& source, DigraphVertex& parent) {
			DigraphVertex v;
			if (mSources.tryFind(source, &v))
				return graph().desc(v).owner.as<ContentType>();
			else {
				NodeType type = NODE_TYPE(CONTENT_BASE_TYPE(ContentType));

				auto factory = mTypeToFactory[type];
				auto ref = factory->load(source);
				v = graph().addNode<CONTENT_BASE_TYPE(ContentType)>(ref, parent);
				mSources.set(v, source);
				return ref.as<ContentType>();
			}
		}

		template <typename ContentType>
		inline DigraphVertex loadNode(const std::string& source, const NodeHandle parentHandle) {
			DigraphVertex v = graph()[parentHandle];
			return loadNode<ContentType>(source, v);
		}

		template <typename ContentType>
		inline ref<ContentType> loadRef(const std::string& source, const NodeHandle parentHandle) {
			DigraphVertex v = graph()[parentHandle];
			return loadRef<ContentType>(source, v);
		}

		template <typename ContentType> 
		inline DigraphVertex loadNode(const std::string& source) {
			// Set self as the parent of the new object
			return loadNode<ContentType>(source, mHandle);
		}

		template <typename ContentType>
		inline ref<ContentType> loadRef(const std::string& source) {
			// Set self as the parent of the new object
			return loadRef<ContentType>(source, mHandle);
		}

		void unloadNode(DigraphVertex& node) {
			auto desc = graph().desc(node);
			// Get factory from type
			auto factory = mTypeToFactory[desc.type];
			if (factory != nullptr)
				factory->unload(desc.owner);
		}

		void unloadAll() {
			for (auto it = graph()[mHandle].getOutgoingNeighbors(); it.valid(); it.next()) {
				DigraphVertex v = it();
				unloadNode(v);
			}
		}

		void dispose() override {
			unloadAll();
		}
	};
}