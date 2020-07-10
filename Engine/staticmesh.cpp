#include "staticmesh.hpp"
#include "json.hpp"

#include <fstream>
#include <iostream>

using namespace std;
using namespace nlohmann;

namespace Morpheus {
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
		delete ref.getAs<StaticMesh>();
	}
	void ContentFactory<StaticMesh>::dispose() {
	}
}