#include "content.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "staticmesh.hpp"
#include "halfedge.hpp"
#include "halfedgeloader.hpp"
#include "texture2d.hpp"

namespace Morpheus {

	void ContentManager::init(Node node) {
		// Make shader factory
		addFactory<Shader>();
		// Make geometry factory
		addFactory<Geometry>();
		// Make material factory
		addFactory<Material>();
		// Make the static mesh factory
		addFactory<StaticMesh>();
		// Make the half edge geometry factory
		addFactory<HalfEdgeGeometry>();
		// Make the Texture2D factory
		addFactory<Texture2D>();

		mSources = graph()->createTwoWayVertexLookup<std::string>("__content__");
		mHandle = graph()->issueHandle(node);
	}

	ContentManager::ContentManager() : mHandle(HANDLE_INVALID) {
	}

	void ContentManager::unload(Node& node) {
		auto desc = graph()->desc(node);
		// Get factory from type
		auto factory = mTypeToFactory[desc->type];
		if (factory)
			factory->unload(desc->owner);
		graph()->deleteVertex(node);
		mSources.remove(node);
	}

	void ContentManager::unloadAll() {
		for (auto it = (*graph())[mHandle].children(); it.valid();) {
			Node v = it();
			it.next();
			unload(v);
		}
	}

	void ContentManager::setSource(const Node& node, const std::string& source) {
		mSources.set(node, source);
	}

	void ContentManager::dispose() {
		unloadAll();

		for (auto& factory : mFactories)
			factory->dispose();
		mFactories.clear();
		mTypeToFactory.clear();

		graph()->destroyLookup(mSources);
		graph()->recallHandle(mHandle);

		delete this;
	}

	void ContentManager::collectGarbage() {
		std::stack<Node> toCollect;

		// Iterate through children and calculate degrees
		for (auto it = node().children(); it.valid(); it.next()) {
			auto degree = it().inDegree();

			// The only parent of this object is the content manager, collect it
			if (degree == 1)
				toCollect.push(it());
		}
		
		// Repeatedly collect until there is nothing to collect
		while (!toCollect.empty()) {

			auto top = toCollect.top();
			toCollect.pop();

			auto type = desc(top)->type;
			std::cout << "Collecting " << nodeTypeString(type) << std::endl;

			// Check if children of this node need to be collected too
			for (auto it = top.children(); it.valid(); it.next()) {
				auto child = it();
				auto newDegree = child.inDegree() - 1;
				if (newDegree <= 1)
					toCollect.push(it());
			}

			// Collect garbage
			unload(top);
		}
	}
}