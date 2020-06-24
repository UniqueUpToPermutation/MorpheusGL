#pragma once

#include "core.hpp"
#include "pool.hpp"

#include <set>

#define REGISTER_CONTENT_BASE_TYPE(subType, baseType) template <typename SubType> struct ContentSubTypeToBaseType { \
	public: \
	typedef baseType Base; \
	}; \

#define CONTENT_BASE_TYPE(subType) ContentSubTypeToBaseType<subType>::baseType

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
	public:
		virtual OwnerRef load(const std::string& source) = 0;
		virtual void unload(OwnerRef& ref) = 0;
		virtual void setHandle(const NodeHandle handle) = 0;
		virtual NodeHandle getHandle() const = 0;
	};
	REGISTER_NODE_TYPE(IContentFactory, NodeType::CONTENT_FACTORY);

	template <typename ContentType>
	class ContentFactory : public IContentFactory {
	};

	class ContentManager {
	private:
		NodeHandle handle_;
		std::set<IContentFactory*> factories_;
		std::unordered_map<NodeType, IContentFactory> typeToFactory_;
		DigraphVertexLookupView<std::string> sources_;

	public:
		NodeHandle handle() const { return handle_; }

		ContentManager() {
			auto v = graph().addNode(this, engine().handle());
			handle_ = graph().issueHandle(v);
			sources_ = graph().createVertexLookup<std::string>("content_src");
		}

		template <typename ContentType> void addFactory() {
			IContentFactory* factory = new ContentFactory<ContentType>(this);
			factories_.insert(factory);
			typeToFactory_[NODE_TYPE(ContentType)] = factory;
			auto v = graph().addNode<IContentFactory>(factory, handle_);
			factory->setHandle(graph().issueHandle(v));
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
		DigraphVertex load(const std::string& source, const DigraphVertex& parent) {
			DigraphVertex v;
			if (sources_.tryFind(source, &v))
				return v;
			else {
				NodeType type = NODE_TYPE(CONTENT_BASE_TYPE(ContentType));

				auto factory = factories_[type];
				auto ref = factory->load(source);
				v = graph().addNode<CONTENT_BASE_TYPE(ContentType)>(ref, parent);
				sources_.set(v, source);
				return v;
			}
		}

		template <typename ContentType>
		inline DigraphVertex load(const std::string& source, const NodeHandle parentHandle) {
			return load<ContentType>(source, graph()[parentHandle]);
		}

		template <typename ContentType> 
		inline DigraphVertex load(const std::string& source) {
			// Set self as the parent of the new object
			return load<ContentType>(source, handle_);
		}

		template <typename ContentType>
		void unload(DigraphVertex node) {
			auto desc = graph().desc(node);
			// Get factory from type
			auto factory = typeToFactory_[desc.type];
			factory.unload(desc.owner);
		}
	};
}