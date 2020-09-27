#include <engine/content.hpp>
#include <engine/shader.hpp>
#include <engine/material.hpp>
#include <engine/geometry.hpp>
#include <engine/staticmesh.hpp>
#include <engine/halfedge.hpp>
#include <engine/halfedgeloader.hpp>
#include <engine/texture.hpp>
#include <engine/sampler.hpp>

namespace Morpheus {

	void ContentManager::init() {
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
		// Make the Texture factory
		addFactory<Texture>();
		// Make the Sampler factory
		addFactory<Sampler>();

		mSources = graph()->createTwoWayVertexLookup<std::string>("__content__");
	}

	ContentManager::ContentManager() : INodeOwner(NodeType::CONTENT_MANAGER) {
	}

	ContentManager::~ContentManager() {
		unloadAll();

		for (auto& factory : mFactories)
			factory->dispose();
		mFactories.clear();
		mTypeToFactory.clear();

		graph()->destroyLookup(mSources);
	}

	void ContentManager::unloadMarked() {
		while (!mMarkedNodes.empty()) {

			auto top = mMarkedNodes.top();
			mMarkedNodes.pop();

			auto type = top->getType();
			std::cout << "Collecting " << nodeTypeString(type) << std::endl;

			if (top->parentCount() <= 1) {
				// Check if children of this node need to be collected too
				for (auto it = top->children(); it.valid(); it.next()) {
					mMarkedNodes.push(it());
				}

				// Collect garbage
				unload(top);
			}
		}
	}

	void ContentManager::unload(INodeOwner* node) {
		// Get factory from type
		auto factory = mTypeToFactory[node->getType()];
		if (factory) {
			graph()->deleteVertex(node->node());
			factory->unload(node);
		}

		std::string src;
		if (mSources.tryFind(node->node(), &src))
			std::cout << "Unloading " << src << " (" << 
				factory->getContentTypeString() << ")..." << std::endl;
		else
			std::cout << "Unloading [UNNAMED] (" << 
				factory->getContentTypeString()  << ")..." << std::endl;

		mSources.remove(node->node());
	}

	void ContentManager::unloadAll() {
		for (auto it = children(); it.valid();) {
			INodeOwner* v = it();
			it.next();
			unload(v);
		}
	}
	
	void ContentManager::collectGarbage() {
		std::stack<INodeOwner*> toCollect;

		// Iterate through children and calculate degrees
		for (auto it = children(); it.valid(); it.next()) {
			auto degree = it()->inDegree();

			// The only parent of this object is the content manager, collect it
			if (degree == 1)
				toCollect.push(it());
		}
		
		// Repeatedly collect until there is nothing to collect
		while (!toCollect.empty()) {

			auto top = toCollect.top();
			toCollect.pop();

			auto type = top->getType();
			std::cout << "Collecting " << nodeTypeString(type) << std::endl;

			// Check if children of this node need to be collected too
			for (auto it = top->children(); it.valid(); it.next()) {
				auto child = it();
				auto newDegree = child->inDegree() - 1;
				if (newDegree <= 1)
					toCollect.push(it());
			}

			// Collect garbage
			unload(top);
		}

		// All marked nodes have been dealt with
		mMarkedNodes = std::stack<INodeOwner*>();
	}
}