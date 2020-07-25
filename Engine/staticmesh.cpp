#include "staticmesh.hpp"
#include "json.hpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace nlohmann;

#define GEOMETRY_PART_INDEX 1
#define MATERIAL_PART_INDEX 0

namespace Morpheus {
	ref<Geometry> StaticMesh::getGeometry(Node meshNode) {
		auto desc_ = desc(getGeometryNode(meshNode));
		assert(desc_->type == NodeType::MATERIAL);
		return desc_->owner.reinterpret<Geometry>();
	}
	Node StaticMesh::getGeometryNode(Node meshNode) {
		auto n = meshNode.getChild(GEOMETRY_PART_INDEX);
		assert(n.valid());
		return n;
	}
	ref<Material> StaticMesh::getMaterial(Node meshNode) {
		auto desc_ = desc(getMaterialNode(meshNode));
		assert(desc_->type == NodeType::MATERIAL);
		return desc_->owner.reinterpret<Material>();
	}
	Node StaticMesh::getMaterialNode(Node meshNode) {
		auto n = meshNode.getChild(MATERIAL_PART_INDEX);
		assert(n.valid());
		return n;
	}
	void StaticMesh::getParts(Node meshNode, ref<Geometry>* geo_out, ref<Material>* mat_out) {
		auto it = meshNode.children();
		assert(it.valid());
		auto desc_ = desc(it());
		assert(desc_->type == NodeType::MATERIAL);
		*mat_out = desc_->owner.reinterpret<Material>();
		it.next();
		assert(it.valid());
		desc_ = desc(it());
		assert(desc_->type == NodeType::GEOMETRY);
		*geo_out = desc_->owner.reinterpret<Geometry>();
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
		delete this;
	}
	Node ContentFactory<StaticMesh>::makeStaticMesh(const Node& material, 
		const Node& geometry, ref<StaticMesh>* refOut)
	{
		return makeStaticMesh(material, geometry, "", refOut);
	}

	Node ContentFactory<StaticMesh>::makeStaticMesh(const Node& material, const Node& geometry,
		const std::string& source,
		ref<StaticMesh>* refOut) {
		assert(graph()->desc(geometry)->type == NodeType::GEOMETRY);
		assert(graph()->desc(material)->type == NodeType::MATERIAL);

		StaticMesh* mesh = new StaticMesh();

		if (refOut)
			*refOut = ref<StaticMesh>(mesh);

		Node staticMeshNode = graph()->addNode(mesh);

		// Add those proxies as children to the static mesh
		graph()->createEdge(staticMeshNode, geometry);
		graph()->createEdge(staticMeshNode, material);

		content()->addContentNode(staticMeshNode, source);
		return staticMeshNode;
	}
}