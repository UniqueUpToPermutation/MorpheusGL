#include <engine/staticmesh.hpp>
#include <engine/json.hpp>

#include <fstream>
#include <iostream>

using namespace std;
using namespace nlohmann;

#define GEOMETRY_PART_INDEX 0
#define MATERIAL_PART_INDEX 1

namespace Morpheus {
	StaticMesh* StaticMesh::toStaticMesh() {
		return this;
	}

	void StaticMesh::setGeometry(Geometry* geo) {
		if (mGeometry) {
			removeChild(mGeometry);
			markForUnload(mGeometry);
		}

		addChild(geo);
		mGeometry = geo;
	}
	void StaticMesh::setMaterial(Material* mat) {
		if (mMaterial) {
			removeChild(mMaterial);
			markForUnload(mMaterial);
		}

		addChild(mat);
		mMaterial = mat;
	}

	INodeOwner* ContentFactory<StaticMesh>::load(const std::string& source, Node loadInto) {
		std::cout << "Loading static mesh " << source << "..." << std::endl;

		ifstream f(source);

		if (!f.is_open()) {
			cout << "Error: failed to open " << source << "!" << endl;
			return nullptr;
		}

		json j;
		f >> j;
		f.close();

		string materialSrc;
		string geometrySrc;

		j["material"].get_to(materialSrc);
		j["geometry"].get_to(geometrySrc);

		string prefix_include_path = "";

		auto extract_ptr = source.find_last_of("\\/");
		if (extract_ptr != string::npos)
			prefix_include_path = source.substr(0, extract_ptr + 1);

		materialSrc = prefix_include_path + materialSrc;
		geometrySrc = prefix_include_path + geometrySrc;

		// Load material and geometry
		Material* material = content()->load<Material>(materialSrc);
		Geometry* geometry = content()->load<Geometry>(geometrySrc);

		// Create proxies for geometry and material
		loadInto.addChild(material->node());
		loadInto.addChild(geometry->node());

		auto staticMesh = new StaticMesh();
		staticMesh->mGeometry = geometry;
		staticMesh->mMaterial = material;

		return staticMesh;
	}

	void ContentFactory<StaticMesh>::unload(INodeOwner* ref) {
		delete ref;
	}

	ContentFactory<StaticMesh>::~ContentFactory() {
	}
	
	std::string ContentFactory<StaticMesh>::getContentTypeString() const {
		return MORPHEUS_STRINGIFY(StaticMesh);
	}

	StaticMesh* ContentFactory<StaticMesh>::makeStaticMesh(Material* material, 
		Geometry* geometry)
	{
		return makeStaticMesh(material, geometry, "");
	}

	StaticMesh* ContentFactory<StaticMesh>::makeStaticMesh(Material* material, Geometry* geometry,
		const std::string& source) {
		assert(geometry->getType() == NodeType::GEOMETRY);
		assert(material->getType() == NodeType::MATERIAL);

		StaticMesh* mesh = new StaticMesh();
		mesh->mGeometry = geometry;
		mesh->mMaterial = material;

		createContentNode(mesh, source);

		// Add material and geometry as children to the static mesh
		mesh->addChild(material);
		mesh->addChild(geometry);

		return mesh;
	}
}