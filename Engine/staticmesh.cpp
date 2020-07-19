#include "staticmesh.hpp"
#include "json.hpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace nlohmann;

namespace Morpheus {
	ref<Geometry> StaticMesh::getGeometry(Node& meshNode) {
		Node material = StaticMesh::getMaterialNode(meshNode);
		auto& graph_ = *graph();
		auto it = material.children();
		NodeData* desc = nullptr;
		for (; it.valid(); it.next()) {
			desc = graph_.desc(it());
			if (graph_.desc(it())->type == NodeType::GEOMETRY_PROXY)
				break;
		}
		if (it.valid())
			return desc->owner.reinterpretGet<Geometry>();
		else
			return ref<Geometry>(nullptr);
	}
	Node StaticMesh::getGeometryNode(Node& meshNode) {
		Node material = StaticMesh::getMaterialNode(meshNode);
		auto& graph_ = *graph();
		auto it = material.children();
		for (; it.valid(); it.next())
			if (graph_.desc(it())->type == NodeType::GEOMETRY_PROXY)
				break;
		if (it.valid())
			return it();
		else
			return Node::invalid();
	}
	ref<Material> StaticMesh::getMaterial(Node& meshNode) {
		auto& graph_ = *graph();
		auto it = meshNode.children();
		NodeData* desc = nullptr;
		for (; it.valid(); it.next()) {
			desc = graph_.desc(it());
			if (desc->type == NodeType::MATERIAL_PROXY)
				break;
		}

		if (it.valid())
			return desc->owner.reinterpretGet<Material>();
		else
			return ref<Material>(nullptr);
	}
	Node StaticMesh::getMaterialNode(Node& meshNode) {
		auto& graph_ = *graph();
		auto it = meshNode.children();
		for (; it.valid(); it.next())
			if (graph_.desc(it())->type == NodeType::MATERIAL_PROXY)
				break;
		if (it.valid())
			return it();
		else
			return Node::invalid();
	}

	ref<void> ContentFactory<StaticMesh>::load(const std::string& source, Node& loadInto) {
		ifstream f(source);

		if (!f.is_open()) {
			cout << "Error: failed to open " << source << "!" << endl;
			return ref<void>(nullptr);
		}

		json j;
		f >> j;
		f.close();

		string materialSrc;
		string geometrySrc;

		j["material"].get_to(materialSrc);
		j["geometry"].get_to(geometrySrc);

		// Load material and geometry
		Node materialNode = content()->load<Material>(materialSrc);
		Node geometryNode = content()->load<Geometry>(geometrySrc);

		// Create proxies for geometry and material
		Node materialProxy = graph()->makeContentProxy(materialNode);
		Node geometryProxy = graph()->makeContentProxy(geometryNode);

		graph()->createEdge(loadInto, materialProxy);
		graph()->createEdge(materialProxy, geometryProxy);
		return ref<void>(new StaticMesh());
	}
	void ContentFactory<StaticMesh>::unload(ref<void>& ref) {
		delete ref.reinterpret<StaticMesh>().get();
	}
	void ContentFactory<StaticMesh>::dispose() {
	}
	Node ContentFactory<StaticMesh>::makeStaticMesh(const Node& material, 
		const Node& geometry, ref<StaticMesh>* refOut)
	{
		assert(graph()->desc(geometry)->type == NodeType::GEOMETRY);
		assert(graph()->desc(material)->type == NodeType::MATERIAL);

		StaticMesh* mesh = new StaticMesh();

		if (refOut)
			*refOut = ref<StaticMesh>(mesh);

		Node staticMeshNode = graph()->addNode(mesh);

		// Create proxies for geometry and material
		Node materialProxy = graph()->makeContentProxy(material);
		Node geometryProxy = graph()->makeContentProxy(geometry);

		// Add those proxies as children to the static mesh
		graph()->createEdge(staticMeshNode, materialProxy);
		graph()->createEdge(materialProxy, geometryProxy);

		return staticMeshNode;
	}
}