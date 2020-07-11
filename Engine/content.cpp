#include "content.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "geometry.hpp"
#include "staticmesh.hpp"

namespace Morpheus {
	ContentManager::ContentManager() {
		// Make shader factory
		addFactory<Shader>();
		// Make geometry factory
		addFactory<Geometry>();
		// Make material factory
		addFactory<Material>();
		// Make the static mesh factory
		addFactory<StaticMesh>();

		mSources = graph()->createVertexLookup<std::string>("__content__");
	}

	void ContentManager::unload(Node& node) {
		auto desc = graph()->desc(node);
		// Get factory from type
		auto factory = mTypeToFactory[desc->type];
		if (factory)
			factory->unload(desc->owner);
		graph()->deleteVertex(node);
	}

	void ContentManager::unloadAll() {
		for (auto it = (*graph())[mHandle].getChildren(); it.valid();) {
			Node v = it();
			it.next();
			unload(v);
		}
	}

	void ContentManager::dispose() {
		unloadAll();

		graph()->destroyLookup(mSources);
	}

	void ContentManager::collectGarbage() {
		DigraphSparseDataView<uint32_t> degrees = graph()->createSparseVertexData<uint32_t>(0, "__content__manager__degree__data__");

		std::stack<Node> toCollect;

		// Iterate through children and calculate degrees
		for (auto it = node().getChildren(); it.valid(); it.next()) {
			auto degree = it().inDegree();
			degrees.set(it(), degree);

			// The only parent of this object is the content manager, collect it
			if (degree == 1)
				toCollect.push(it());
		}
		
		// Repeatedly collect until there is nothing to collect
		while (!toCollect.empty()) {
			auto top = toCollect.top();
			toCollect.pop();

			// Check if children of this node need to be collected too
			for (auto it = top.getChildren(); it.valid(); it.next()) {
				auto newDegree = degrees.get(it()) - 1;
				degrees.set(it(), newDegree);
				if (newDegree == 1)
					toCollect.push(it());
			}

			// Collect garbage
			unload(top);
		}

		graph()->destroyData(degrees);
	}
}