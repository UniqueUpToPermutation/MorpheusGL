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

	class IContentFactory {
	private:
		NodeHandle handle_;

	public:
		virtual OwnerRef load(const std::string& source) = 0;
		virtual void unload(OwnerRef& ref) = 0;
		inline NodeHandle handle() const { return handle_; }

		friend class ContentManager;
	};
	REGISTER_NODE_TYPE(IContentFactory, NodeType::CONTENT_FACTORY);
	REGISTER_CONTENT_BASE_TYPE(IContentFactory, IContentFactory);

	template <typename ContentType>
	class ContentFactory : public IContentFactory {
	};

	class ContentManager {
	private:
		NodeHandle handle_;
		std::set<IContentFactory*> factories_;
		std::unordered_map<NodeType, IContentFactory*> typeToFactory_;
		DigraphVertexLookupView<std::string> sources_;

	public:
		NodeHandle handle() const { return handle_; }

		ContentManager();

		template <typename ContentType> void addFactory() {
			IContentFactory* factory = new ContentFactory<ContentType>();
			factories_.insert(factory);
			typeToFactory_[NODE_TYPE(ContentType)] = factory;
			auto v = graph().addNode<IContentFactory>(factory, handle_);
			factory->handle_ = graph().issueHandle(v);
		}

		template <typename ContentType> void removeFactory() {
			auto type = NODE_TYPE(ContentType);
			auto factory = typeToFactory_[type];
			factories_.erase(factory);
			typeToFactory_.erase(type);
			graph().recallHandle(factory->getHandle());
			delete factory;
		}

		template <typename ContentType>
		DigraphVertex load(const std::string& source, DigraphVertex& parent) {
			DigraphVertex v;
			if (sources_.tryFind(source, &v))
				return v;
			else {
				NodeType type = NODE_TYPE(CONTENT_BASE_TYPE(ContentType));

				auto factory = typeToFactory_[type];
				auto ref = factory->load(source);
				v = graph().addNode<CONTENT_BASE_TYPE(ContentType)>(ref, parent);
				sources_.set(v, source);
				return v;
			}
		}

		template <typename ContentType>
		inline DigraphVertex load(const std::string& source, const NodeHandle parentHandle) {
			DigraphVertex v = graph()[parentHandle];
			return load<ContentType>(source, v);
		}

		template <typename ContentType> 
		inline DigraphVertex load(const std::string& source) {
			// Set self as the parent of the new object
			return load<ContentType>(source, handle_);
		}

		void unload(DigraphVertex& node) {
			auto desc = graph().desc(node);
			// Get factory from type
			auto factory = typeToFactory_[desc.type];
			if (factory != nullptr)
				factory->unload(desc.owner);
		}

		void unloadAll() {
			for (auto it = graph()[handle_].getOutgoingNeighbors(); it.valid(); it.next()) {
				DigraphVertex v = it();
				unload(v);
			}

			for (auto& it : factories_)
				delete it;
		}
	};
}