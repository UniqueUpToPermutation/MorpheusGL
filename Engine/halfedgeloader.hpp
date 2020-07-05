#pragma once

#include <string>

namespace Assimp {
	class Importer;
}

namespace Morpheus {
	class HalfEdgeGeometry;

	class HalfEdgeLoader {
	private:
		Assimp::Importer* mImporter;

	public:
		HalfEdgeGeometry* load(const std::string& source);

		HalfEdgeLoader();
		~HalfEdgeLoader();
	};
}